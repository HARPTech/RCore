#ifndef LRT_LIBRCP_MESSAGE_H
#define LRT_LIBRCP_MESSAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

  /* Encodes the message type. The rest of the bits is held free for an inner
   * sequence number for dropping old messages in fast paced environments (many
   * updates per second). LiteComm Sequence numbers have to be handled by the
   * application, because it is not certain if data is important or can be
   * thrown away. */
  typedef enum lrt_rcp_message_type_t
  {
    LRT_RCP_MESSAGE_TYPE_UPDATE = 0b00000000,
    LRT_RCP_MESSAGE_TYPE_REQUEST = 0b10000000,
    LRT_RCP_MESSAGE_TYPE_SUBSCRIBE = 0b01000000,
    LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE = 0b11000000,
  } lrt_rcp_message_type_t;

#ifdef BIG_ENDIAN
#define LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, sTYPENAME, tTYPE) _Pragma("error")
#else
#define LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, sTYPENAME, tTYPE)             \
  /* This union wraps the specified type for conversion into bytes. */   \
  union sPREFIX##__##tTYPE##_t                                           \
  {                                                                      \
    tTYPE data;                                                          \
    uint8_t bytes[sizeof(tTYPE)];                                        \
  };                                                                     \
  static union sPREFIX##__##tTYPE##_t sPREFIX##__union_##tTYPE;          \
                                                                         \
  int16_t sPREFIX##_set_data_##sTYPENAME(                                \
    sPREFIX##_block_t* block, tTYPE data, int16_t counter)               \
  {                                                                      \
    if(counter < 0) {                                                    \
      counter = sizeof(tTYPE);                                           \
    }                                                                    \
    sPREFIX##__union_##tTYPE.data = data;                                \
    for(size_t i = 1; counter > 0 && i < sPREFIX##_get_data_size(block); \
        ++i, --counter) {                                                \
      sPREFIX##_set_data(                                                \
        block, i, sPREFIX##__union_##tTYPE.bytes[counter - 1]);          \
    }                                                                    \
    return counter;                                                      \
  }                                                                      \
  int16_t sPREFIX##_get_data_##sTYPENAME(                                \
    sPREFIX##_block_t* block, tTYPE* data, int16_t counter)              \
  {                                                                      \
    if(counter < 0) {                                                    \
      counter = sizeof(tTYPE);                                           \
      *data = 0;                                                         \
      sPREFIX##__union_##tTYPE.data = 0;                                 \
      sPREFIX##_set_sStart(block, true);                                 \
    } else {                                                             \
      sPREFIX##__union_##tTYPE.data = *data;                             \
    }                                                                    \
    for(size_t i = 1; counter > 0 && i < sPREFIX##_get_data_size(block); \
        ++i, --counter) {                                                \
      sPREFIX##__union_##tTYPE.bytes[counter - 1] =                      \
        sPREFIX##_get_data(block, i);                                    \
    }                                                                    \
    *data = sPREFIX##__union_##tTYPE.data;                               \
    if(counter == 0)                                                     \
      sPREFIX##_set_sEnd(block, true);                                   \
    return counter;                                                      \
  }

#endif

#define LRT_LIBRCP_TYPES(sPREFIX)                                          \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Bool, bool)                           \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint8, uint8_t)                       \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int8, int8_t)                         \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint16, uint16_t)                     \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int16, int16_t)                       \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint32, uint32_t)                     \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int32, int32_t)                       \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint64, uint64_t)                     \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int64, int64_t)                       \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Float, float)                         \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Double, double)                       \
  lrt_rcp_message_type_t sPREFIX##_get_litecomm_message_type(              \
    sPREFIX##_block_t* block)                                              \
  {                                                                        \
    return (lrt_rcp_message_type_t)(sPREFIX##_get_data(block, 0) &         \
                                    0b11000000u);                          \
  }                                                                        \
  void sPREFIX##_set_litecomm_message_type(sPREFIX##_block_t* block,       \
                                           lrt_rcp_message_type_t type)    \
  {                                                                        \
    sPREFIX##_set_data(                                                    \
      block, 0, (sPREFIX##_get_data(block, 0) & 0b00111111u) | type);      \
  }                                                                        \
  uint8_t sPREFIX##_get_litecomm_sequence_number(sPREFIX##_block_t* block) \
  {                                                                        \
    return sPREFIX##_get_data(block, 0) & 0b00111111u;                     \
  }                                                                        \
  void sPREFIX##_set_litecomm_sequence_number(sPREFIX##_block_t* block,    \
                                              uint8_t seq)                 \
  {                                                                        \
    sPREFIX##_set_data(block,                                              \
                       0,                                                  \
                       sPREFIX##_get_litecomm_message_type(block) |        \
                         (seq & 0b00111111u));                             \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
