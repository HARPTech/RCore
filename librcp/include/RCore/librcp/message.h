#ifndef LRT_LIBRCP_MESSAGE_H
#define LRT_LIBRCP_MESSAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __STDC_NO_THREADS__
#define THREAD_LOCAL
#else
#include <threads.h>
#define THREAD_LOCAL thread_local
#endif

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

#define LRT_RCP_MESSAGE_TYPE_COUNT 4

  inline const char* lrt_rcp_message_type_to_str(lrt_rcp_message_type_t msg)
  {
    switch(msg) {
      case LRT_RCP_MESSAGE_TYPE_UPDATE:
        return "Update";
      case LRT_RCP_MESSAGE_TYPE_REQUEST:
        return "Request";
      case LRT_RCP_MESSAGE_TYPE_SUBSCRIBE:
        return "Subscribe";
      case LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE:
        return "Unsubscribe";
      default:
        return "Unknown Message Type";
    }
  }

  /* Conversion Unions
   * -----------------------------------------------------------------
   */

#define LRT_LIBRCP_CONVERSION_UNION(sTYPENAME, tTYPE)               \
  union lrt_librcp_##sTYPENAME##_t                                  \
  {                                                                 \
    tTYPE val;                                                      \
    uint8_t bytes[sizeof(tTYPE)];                                   \
  };                                                                \
  static THREAD_LOCAL union lrt_librcp_##sTYPENAME##_t              \
    lrt_librcp_union_##sTYPENAME;                                   \
  const uint8_t* lrt_librcp_##sTYPENAME##_to_data(const tTYPE val); \
  tTYPE lrt_librcp_##sTYPENAME##_from_data(const uint8_t* data, size_t length);

  LRT_LIBRCP_CONVERSION_UNION(Bool, bool)
  LRT_LIBRCP_CONVERSION_UNION(Uint8, uint8_t)
  LRT_LIBRCP_CONVERSION_UNION(Int8, int8_t)
  LRT_LIBRCP_CONVERSION_UNION(Uint16, uint16_t)
  LRT_LIBRCP_CONVERSION_UNION(Int16, int16_t)
  LRT_LIBRCP_CONVERSION_UNION(Uint32, uint32_t)
  LRT_LIBRCP_CONVERSION_UNION(Int32, int32_t)
  LRT_LIBRCP_CONVERSION_UNION(Uint64, uint64_t)
  LRT_LIBRCP_CONVERSION_UNION(Int64, int64_t)
  LRT_LIBRCP_CONVERSION_UNION(Float, float)
  LRT_LIBRCP_CONVERSION_UNION(Double, double)

  /* Block conversions
   * -----------------------------------------------------------------
   */

#ifdef BIG_ENDIAN
#define LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, sTYPENAME, tTYPE)             \
  int16_t sPREFIX##_set_data_##sTYPENAME(                                \
    sPREFIX##_block_t* block, tTYPE data, int16_t counter)               \
  {                                                                      \
    if(counter < 0) {                                                    \
      counter = sizeof(tTYPE);                                           \
    }                                                                    \
    lrt_librcp_union_##sTYPENAME.val = data;                             \
    for(size_t i = 1; counter > 0 && i < sPREFIX##_get_data_size(block); \
        ++i, --counter) {                                                \
      sPREFIX##_set_data(                                                \
        block,                                                           \
        i,                                                               \
        lrt_librcp_union_##sTYPENAME.bytes[sizeof(tTYPE) - counter]);    \
    }                                                                    \
    return counter;                                                      \
  }                                                                      \
  int16_t sPREFIX##_get_data_##sTYPENAME(                                \
    sPREFIX##_block_t* block, tTYPE* data, int16_t counter)              \
  {                                                                      \
    if(counter < 0) {                                                    \
      counter = sizeof(tTYPE);                                           \
      *data = 0;                                                         \
      lrt_librcp_union_##sTYPENAME.val = 0;                              \
      sPREFIX##_set_sStart(block, true);                                 \
    } else {                                                             \
      lrt_librcp_union_##sTYPENAME.val = *data;                          \
    }                                                                    \
    for(size_t i = 1; counter > 0 && i < sPREFIX##_get_data_size(block); \
        ++i, --counter) {                                                \
      lrt_librcp_union_##sTYPENAME.bytes[sizeof(tTYPE) - counter] =      \
        sPREFIX##_get_data(block, i);                                    \
    }                                                                    \
    *data = lrt_librcp_union_##sTYPENAME.val;                            \
    if(counter == 0)                                                     \
      sPREFIX##_set_sEnd(block, true);                                   \
    return counter;                                                      \
  }
#else
#define LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, sTYPENAME, tTYPE)             \
  int16_t sPREFIX##_set_data_##sTYPENAME(                                \
    sPREFIX##_block_t* block, tTYPE data, int16_t counter)               \
  {                                                                      \
    if(counter < 0) {                                                    \
      counter = sizeof(tTYPE);                                           \
    }                                                                    \
    lrt_librcp_union_##sTYPENAME.val = data;                             \
    for(size_t i = 1; counter > 0 && i < sPREFIX##_get_data_size(block); \
        ++i, --counter) {                                                \
      sPREFIX##_set_data(                                                \
        block, i, lrt_librcp_union_##sTYPENAME.bytes[counter - 1]);      \
    }                                                                    \
    return counter;                                                      \
  }                                                                      \
  int16_t sPREFIX##_get_data_##sTYPENAME(                                \
    sPREFIX##_block_t* block, tTYPE* data, int16_t counter)              \
  {                                                                      \
    if(counter < 0) {                                                    \
      counter = sizeof(tTYPE);                                           \
      *data = 0;                                                         \
      lrt_librcp_union_##sTYPENAME.val = 0;                              \
      sPREFIX##_set_sStart(block, true);                                 \
    } else {                                                             \
      lrt_librcp_union_##sTYPENAME.val = *data;                          \
    }                                                                    \
    for(size_t i = 1; counter > 0 && i < sPREFIX##_get_data_size(block); \
        ++i, --counter) {                                                \
      lrt_librcp_union_##sTYPENAME.bytes[counter - 1] =                  \
        sPREFIX##_get_data(block, i);                                    \
    }                                                                    \
    *data = lrt_librcp_union_##sTYPENAME.val;                            \
    if(counter == 0)                                                     \
      sPREFIX##_set_sEnd(block, true);                                   \
    return counter;                                                      \
  }

#endif

#define LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, sTYPENAME, tTYPE) \
  int16_t sPREFIX##_set_data_##sTYPENAME(                               \
    sPREFIX##_block_t* block, tTYPE data, int16_t counter);             \
  int16_t sPREFIX##_get_data_##sTYPENAME(                               \
    sPREFIX##_block_t* block, tTYPE* data, int16_t counter);

#define LRT_LIBRCP_TYPES_DEFINITIONS(sPREFIX)                               \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Bool, bool)                 \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Uint8, uint8_t)             \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Int8, int8_t)               \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Uint16, uint16_t)           \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Int16, int16_t)             \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Uint32, uint32_t)           \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Int32, int32_t)             \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Uint64, uint64_t)           \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Int64, int64_t)             \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Float, float)               \
  LRT_LIBRCP_TYPES_FOR_TYPE_DEFINITION(sPREFIX, Double, double)             \
  lrt_rcp_message_type_t sPREFIX##_get_litecomm_message_type(               \
    sPREFIX##_block_t* block);                                              \
  void sPREFIX##_set_litecomm_message_type(sPREFIX##_block_t* block,        \
                                           lrt_rcp_message_type_t type);    \
  uint8_t sPREFIX##_get_litecomm_sequence_number(sPREFIX##_block_t* block); \
  void sPREFIX##_set_litecomm_sequence_number(sPREFIX##_block_t* block,     \
                                              uint8_t seq);

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
    sPREFIX##_set_data(block,                                              \
                       0,                                                  \
                       (sPREFIX##_get_data(block, 0) & 0b00111111u) |      \
                         (type & 0b11000000));                             \
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
