#include "../include/RCore/transmit_buffer.h"
#include "../include/RCore/internal/hashtable.h"
#include <RCore/librcp/message.h>
#include <RCore/librsp/stream_message.h>
#include <assert.h>
#include <klib/khash.h>
#include <klib/klist.h>

LRT_LIBRCORE_HASHTABLE_INIT(lrt_rcore_transmit_buffer_hashmap,
                            struct lrt_rcore_transmit_buffer_entry_t)

static void
free_kstring(kstring_t* str)
{
  free(str->s);
}

KMEMPOOL_INIT(lrt_rcore_transmit_buffer_string_mempool,
              kstring_t,
              free_kstring);

typedef struct lrt_rcore_transmit_buffer_t
{
  khash_t(lrt_rcore_transmit_buffer_hashmap) * outgoing;
  khash_t(lrt_rcore_transmit_buffer_hashmap) * incoming;
  kmp_lrt_rcore_transmit_buffer_string_mempool_t* memory_pool;

  void* finished_cb_userdata;
  lrt_rcore_transmit_buffer_finished_cb finished_cb;
  void* data_ready_cb_userdata;
  lrt_rcore_transmit_buffer_data_ready_cb data_ready_cb;
} lrt_rcore_transmit_buffer_t;

lrt_rcore_transmit_buffer_t*
lrt_rcore_transmit_buffer_init()
{
  lrt_rcore_transmit_buffer_t* tr =
    calloc(1, sizeof(lrt_rcore_transmit_buffer_t));

  tr->incoming = kh_init_lrt_rcore_transmit_buffer_hashmap();
  tr->outgoing = kh_init_lrt_rcore_transmit_buffer_hashmap();

  tr->memory_pool = kmp_init_lrt_rcore_transmit_buffer_string_mempool();

  return tr;
}
void
lrt_rcore_transmit_buffer_free(lrt_rcore_transmit_buffer_t* tr)
{
  kh_destroy_lrt_rcore_transmit_buffer_hashmap(tr->incoming);
  kh_destroy_lrt_rcore_transmit_buffer_hashmap(tr->outgoing);
  kmp_destroy_lrt_rcore_transmit_buffer_string_mempool(tr->memory_pool);
}

static struct lrt_rcore_transmit_buffer_entry_t*
get_or_create_entry(lrt_rcore_transmit_buffer_t* origin,
                    kmp_lrt_rcore_transmit_buffer_string_mempool_t* mp,
                    khash_t(lrt_rcore_transmit_buffer_hashmap) * map,
                    LRT_LIBRCORE_HASHTABLE_KEY_TYPE key)
{
  khint_t it = kh_get_lrt_rcore_transmit_buffer_hashmap(map, key);

  if(it == kh_end(map)) {
    int ret = 0;
    it = kh_put_lrt_rcore_transmit_buffer_hashmap(map, key, &ret);

    if(ret == -1) {
      return 0;
    }

    struct lrt_rcore_transmit_buffer_entry_t* entry = &kh_val(map, it);
    entry->data = kmp_alloc_lrt_rcore_transmit_buffer_string_mempool(mp);
    entry->origin = origin;
    entry->reliable = true;
    entry->type = 0;
    entry->property = 0;
    entry->liteCommMessageType = 0;
    entry->transmit_offset = 0;
    return entry;
  }

  struct lrt_rcore_transmit_buffer_entry_t* entry = &kh_val(map, it);

  if(entry->data == 0) {
    entry->data = kmp_alloc_lrt_rcore_transmit_buffer_string_mempool(mp);
  }

  return entry;
}

void
lrt_rcore_transmit_buffer_reserve(lrt_rcore_transmit_buffer_t* handle,
                                  uint8_t type,
                                  uint16_t property,
                                  size_t length)
{
  struct lrt_rcore_transmit_buffer_entry_t* entry =
    get_or_create_entry(handle,
                        handle->memory_pool,
                        handle->incoming,
                        lrt__hashtable_key_from_property(type, property));

  if(entry == 0) {
    return;
  }

  // Reserve the space requested.
  ks_resize(entry->data, ks_len(entry->data) + length);
}

void
lrt_rcore_transmit_buffer_receive_data_byte(lrt_rcore_transmit_buffer_t* handle,
                                            uint8_t type,
                                            uint16_t property,
                                            uint8_t streamBits,
                                            uint8_t byte,
                                            bool reliable)
{
  struct lrt_rcore_transmit_buffer_entry_t* entry =
    get_or_create_entry(handle,
                        handle->memory_pool,
                        handle->incoming,
                        lrt__hashtable_key_from_property(type, property));
  if(entry->data->l == 0) {
    // This is the first call, the entry can be initialised.
    entry->type = type;
    entry->property = property;
    entry->reliable = reliable;
    entry->transmit_offset = 0;
  }

  if(entry == 0) {
    return;
  }

  // Insert new byte.
  kputc(byte, entry->data);

  // Check if this is the last bit (the sEnd flag is set), which would make this
  // message ready to be given to the callback and the entry to be free'd.
  if(streamBits & LRT_LIBRSP_STREAM_END) {
    handle->finished_cb(entry, handle->finished_cb_userdata);

    entry->data->l = 0;

    kmp_free_lrt_rcore_transmit_buffer_string_mempool(handle->memory_pool,
                                                      entry->data);

    // Free the entry.
    khint_t it = kh_get_lrt_rcore_transmit_buffer_hashmap(
      handle->incoming,
      lrt__hashtable_key_from_property(entry->type, entry->property));

    if(it == kh_end(handle->outgoing)) {
      return;
    }

    kh_del_lrt_rcore_transmit_buffer_hashmap(handle->incoming, it);
  }
}

void
lrt_rcore_transmit_buffer_set_finished_cb(
  lrt_rcore_transmit_buffer_t* handle,
  lrt_rcore_transmit_buffer_finished_cb cb,
  void* userdata)
{
  handle->finished_cb_userdata = userdata;
  handle->finished_cb = cb;
}
void
lrt_rcore_transmit_buffer_set_data_ready_cb(
  lrt_rcore_transmit_buffer_t* handle,
  lrt_rcore_transmit_buffer_data_ready_cb cb,
  void* userdata)
{
  handle->data_ready_cb_userdata = userdata;
  handle->data_ready_cb = cb;
}

void
lrt_rcore_transmit_buffer_send_ctrl(lrt_rcore_transmit_buffer_t* handle,
                                    uint8_t type,
                                    uint16_t property,
                                    uint8_t liteCommMessageType,
                                    bool reliable)
{
  struct lrt_rcore_transmit_buffer_entry_t* entry =
    get_or_create_entry(handle,
                        handle->memory_pool,
                        handle->outgoing,
                        lrt__hashtable_key_from_property(type, property));

  if(entry == 0) {
    return;
  }

  // Nothing needs to be resized, the data is only shortened to 0.
  entry->data->l = 0;
  entry->transmit_offset = 0;
  entry->type = type;
  entry->property = property;
  entry->reliable = reliable;
  entry->seq_number = 0;
  entry->liteCommMessageType = liteCommMessageType;

  handle->data_ready_cb(entry, handle->data_ready_cb_userdata);
}

void
lrt_rcore_transmit_buffer_free_send_slot(
  lrt_rcore_transmit_buffer_t* handle,
  lrt_rcore_transmit_buffer_entry_t* entry)
{
  khint_t it = kh_get_lrt_rcore_transmit_buffer_hashmap(
    handle->outgoing,
    lrt__hashtable_key_from_property(entry->type, entry->property));

  if(it == kh_end(handle->outgoing)) {
    return;
  }

  kh_del_lrt_rcore_transmit_buffer_hashmap(handle->outgoing, it);
}

void
lrt_rcore_transmit_buffer_send_data(lrt_rcore_transmit_buffer_t* handle,
                                    uint8_t type,
                                    uint16_t property,
                                    const uint8_t* data,
                                    size_t length,
                                    bool reliable)
{
  struct lrt_rcore_transmit_buffer_entry_t* entry =
    get_or_create_entry(handle,
                        handle->memory_pool,
                        handle->outgoing,
                        lrt__hashtable_key_from_property(type, property));

  if(entry == 0) {
    return;
  }

  // Reserve the required space, then send the data over the callback.
  ks_resize(entry->data, ks_len(entry->data) + length);
  memcpy(entry->data, data, length);
  entry->transmit_offset = 0;
  entry->type = type;
  entry->property = property;
  entry->reliable = reliable;
  entry->seq_number = 0;
  entry->liteCommMessageType = LRT_RCP_MESSAGE_TYPE_UPDATE;

  assert(handle->data_ready_cb != 0);
  handle->data_ready_cb(entry, handle->data_ready_cb_userdata);
}

#define LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(sTYPE, tTYPE)  \
  void lrt_rcore_transmit_buffer_send_##sTYPE(                 \
    lrt_rcore_transmit_buffer_t* handle,                       \
    uint8_t type,                                              \
    uint16_t property,                                         \
    tTYPE value,                                               \
    bool reliable)                                             \
  {                                                            \
    const uint8_t* data = lrt_librcp_##sTYPE##_to_data(value); \
    lrt_rcore_transmit_buffer_send_data(                       \
      handle, type, property, data, sizeof(tTYPE), reliable);  \
  }

LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Bool, bool)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Uint8, uint8_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Int8, int8_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Uint16, uint16_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Int16, int16_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Uint32, uint32_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Int32, int32_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Uint64, uint64_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Int64, int64_t)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Float, float)
LRT_RCORE_TRANSMITBUFFER_SEND_TYPE_IMPL(Double, double)
