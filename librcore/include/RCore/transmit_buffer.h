#ifndef LRT_LIBRCORE_TRANSMITBUFFER_H
#define LRT_LIBRCORE_TRANSMITBUFFER_H

#include "events.h"
#include "internal/hashtable.h"
#include <RCore/librcp/message.h>
#include <klib/kstring.h>

#ifdef __cplusplus
#include <memory>

extern "C"
{
#endif

#include <stddef.h>

#define LRT_RCORE_STACK_CROSSOVER(                                           \
  sPREFIX1, sPREFIX2, iBLOCKSIZE1, iBLOCKSIZE2)                              \
  int16_t lrt_convert_##sPREFIX1##_to_##sPREFIX2(                            \
    sPREFIX1##_block_t* block1, sPREFIX2##_block_t* block2, int16_t counter) \
  {                                                                          \
    if(iBLOCKSIZE1 == iBLOCKSIZE2) {                                         \
      /* Block sizes are equal and the data can be copied directly. Sequence \
       * number has to be set again. */                                      \
      memcpy(&block1->data, &block2->data, iBLOCKSIZE1);                     \
    }                                                                        \
  }

#define LRT_RCORE_STACK_CROSSOVER_DUPLEX(                                 \
  sPREFIX1, sPREFIX2, iBLOCKSIZE1, iBLOCKSIZE2)                           \
  LRT_RCORE_STACK_CROSSOVER(sPREFIX1, sPREFIX2, iBLOCKSIZE1, iBLOCKSIZE2) \
  LRT_RCORE_STACK_CROSSOVER(sPREFIX2, sPREFIX1, iBLOCKSIZE2, iBLOCKSIZE1)

  typedef struct lrt_rcore_transmit_buffer_t lrt_rcore_transmit_buffer_t;

  typedef struct lrt_rcore_transmit_buffer_entry_t
  {
    kstring_t* data;
    uint8_t type;
    uint16_t property;
    uint16_t seq_number;
    bool reliable;
    size_t transmit_offset;
    lrt_rcp_message_type_t message_type;
    lrt_rcore_transmit_buffer_t* origin;
  } lrt_rcore_transmit_buffer_entry_t;

  inline bool lrt_rcore_transmit_buffer_entry_transmit_finished(
    lrt_rcore_transmit_buffer_entry_t* entry)
  {
    return entry->transmit_offset >= entry->data->l;
  }

  typedef lrt_rcore_event_t (*lrt_rcore_transmit_buffer_finished_cb)(
    lrt_rcore_transmit_buffer_entry_t* entry,
    void* userdata);
  typedef lrt_rcore_event_t (*lrt_rcore_transmit_buffer_data_ready_cb)(
    lrt_rcore_transmit_buffer_entry_t* entry,
    void* userdata);

  lrt_rcore_transmit_buffer_t* lrt_rcore_transmit_buffer_init();
  void lrt_rcore_transmit_buffer_free(lrt_rcore_transmit_buffer_t* tr);

  void lrt_rcore_transmit_buffer_reserve(lrt_rcore_transmit_buffer_t* handle,
                                         uint8_t type,
                                         uint16_t property,
                                         size_t length);

  void lrt_rcore_transmit_buffer_receive_data_byte(
    lrt_rcore_transmit_buffer_t* handle,
    uint8_t type,
    uint16_t property,
    uint8_t streamBits,
    uint8_t byte,
    lrt_rcp_message_type_t message_type,
    bool reliable,
    uint16_t seq_number);

  void lrt_rcore_transmit_buffer_free_send_slot(
    lrt_rcore_transmit_buffer_t* handle,
    lrt_rcore_transmit_buffer_entry_t* entry);

  void lrt_rcore_transmit_buffer_send_ctrl(lrt_rcore_transmit_buffer_t* handle,
                                           uint8_t type,
                                           uint16_t property,
                                           lrt_rcp_message_type_t message_type,
                                           bool reliable);

  void lrt_rcore_transmit_buffer_send_data(lrt_rcore_transmit_buffer_t* handle,
                                           uint8_t type,
                                           uint16_t property,
                                           const uint8_t* data,
                                           size_t length,
                                           bool reliable);

#define LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(sTYPE, tTYPE) \
  void lrt_rcore_transmit_buffer_send_##sTYPE(           \
    lrt_rcore_transmit_buffer_t* handle,                 \
    uint8_t type,                                        \
    uint16_t property,                                   \
    tTYPE value,                                         \
    bool reliable);

  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Bool, bool)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Uint8, uint8_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Int8, int8_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Uint16, uint16_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Int16, int16_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Uint32, uint32_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Int32, int32_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Uint64, uint64_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Int64, int64_t)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Float, float)
  LRT_RCORE_TRANSMITBUFFER_SEND_TYPE(Double, double)

  void lrt_rcore_transmit_buffer_set_finished_cb(
    lrt_rcore_transmit_buffer_t* handle,
    lrt_rcore_transmit_buffer_finished_cb cb,
    void* userdata);
  void lrt_rcore_transmit_buffer_set_data_ready_cb(
    lrt_rcore_transmit_buffer_t* handle,
    lrt_rcore_transmit_buffer_data_ready_cb cb,
    void* userdata);

#ifdef __cplusplus
}// closing brace for extern "C"

namespace lrt {
namespace RCore {
struct TransmitBufferPtrDeleter
{
  void operator()(lrt_rcore_transmit_buffer_t* buffer)
  {
    lrt_rcore_transmit_buffer_free(buffer);
  }
};

using TransmitBufferPtr =
  std::unique_ptr<lrt_rcore_transmit_buffer_t, TransmitBufferPtrDeleter>;
}
}
#endif

#endif
