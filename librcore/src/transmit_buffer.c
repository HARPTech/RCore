#include "../include/RCore/internal/hashtable.h"
#include <RCore/librsp/stream_message.h>
#include <assert.h>
#include <klib/khash.h>
#include <klib/klist.h>
#include <klib/kstring.h>

struct lrt_rcore_transmit_buffer_entry_t
{
  kstring_t* data;
};

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

#include "../include/RCore/transmit_buffer.h"

typedef struct lrt_rcore_transmit_buffer_t
{
  khash_t(lrt_rcore_transmit_buffer_hashmap) outgoing;
  khash_t(lrt_rcore_transmit_buffer_hashmap) incoming;
  kmp_lrt_rcore_transmit_buffer_string_mempool_t memory_pool;

  void* finished_cb_userdata;
  lrt_rcore_transmit_buffer_finished_cb finished_cb;
} lrt_rcore_transmit_buffer_t;

static struct lrt_rcore_transmit_buffer_entry_t*
get_or_create_entry(kmp_lrt_rcore_transmit_buffer_string_mempool_t* mp,
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
    get_or_create_entry(&handle->memory_pool,
                        &handle->incoming,
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
                                            uint8_t byte)
{
  struct lrt_rcore_transmit_buffer_entry_t* entry =
    get_or_create_entry(&handle->memory_pool,
                        &handle->incoming,
                        lrt__hashtable_key_from_property(type, property));

  if(entry == 0) {
    return;
  }

  // Insert new byte.
  kputc(byte, entry->data);

  // Check if this is the last bit (the sEnd flag is set), which would make this
  // message ready to be given to the callback and the entry to be free'd.
  if(streamBits & LRT_LIBRSP_STREAM_END) {
    handle->finished_cb((const uint8_t*)entry->data->s,
                        entry->data->l,
                        handle->finished_cb_userdata);

    kmp_free_lrt_rcore_transmit_buffer_string_mempool(&handle->memory_pool,
                                                      entry->data);
    entry = 0;
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
