#include "../include/RCore/librcp/message.h"
#include "../../librsp/include/RCore/librsp/data.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(BIG_ENDIAN) && !defined(__AVR__)
#define LRT_LIBRCP_CONVERSION_IMPL(sTYPENAME, tTYPE)                           \
  tTYPE lrt_librcp_##sTYPENAME##_from_data(const uint8_t* data, size_t length) \
  {                                                                            \
    assert(length <= sizeof(tTYPE));                                           \
    if(lrt_librcp_union_##sTYPENAME.bytes != data) {                           \
      memcpy(lrt_librcp_union_##sTYPENAME.bytes, data, length);                \
    }                                                                          \
    /* Reverse the data for Big Endian byte order. */                          \
    size_t i = sizeof(tTYPE) - 1;                                              \
    size_t j = 0;                                                              \
    while(i > j) {                                                             \
      uint8_t tmp = lrt_librcp_union_##sTYPENAME.bytes[i];                     \
      lrt_librcp_union_##sTYPENAME.bytes[i] =                                  \
        lrt_librcp_union_##sTYPENAME.bytes[j];                                 \
      lrt_librcp_union_##sTYPENAME.bytes[j] = tmp;                             \
      --i;                                                                     \
      ++j;                                                                     \
    }                                                                          \
    return lrt_librcp_union_##sTYPENAME.val;                                   \
  }                                                                            \
  const uint8_t* lrt_librcp_##sTYPENAME##_to_data(const tTYPE val)             \
  {                                                                            \
    lrt_librcp_union_##sTYPENAME.val = val;                                    \
    /* Reverse the data for Big Endian byte order. */                          \
    size_t i = sizeof(tTYPE) - 1;                                              \
    size_t j = 0;                                                              \
    while(i > j) {                                                             \
      uint8_t tmp = lrt_librcp_union_##sTYPENAME.bytes[i];                     \
      lrt_librcp_union_##sTYPENAME.bytes[i] =                                  \
        lrt_librcp_union_##sTYPENAME.bytes[j];                                 \
      lrt_librcp_union_##sTYPENAME.bytes[j] = tmp;                             \
      --i;                                                                     \
      ++j;                                                                     \
    }                                                                          \
    return lrt_librcp_union_##sTYPENAME.bytes;                                 \
  }                                                                            \
  void lrt_librcp_##sTYPENAME##_set_data(lrt_rbp_message_t* message,           \
                                         const tTYPE val)                      \
  {                                                                            \
    rcomm_message_insert_data(                                                 \
      message, lrt_librcp_##sTYPENAME##_to_data(val), sizeof(tTYPE), 0);       \
  }                                                                            \
  tTYPE lrt_librcp_##sTYPENAME##_get_data(lrt_rbp_message_t* message)          \
  {                                                                            \
    rcomm_message_read_data(                                                   \
      message, lrt_librcp_union_##sTYPENAME.bytes, sizeof(tTYPE), 0);          \
    return lrt_librcp_##sTYPENAME##_from_data(                                 \
      lrt_librcp_union_##sTYPENAME.bytes, sizeof(tTYPE));                   \
  }
#else
#define LRT_LIBRCP_CONVERSION_IMPL(sTYPENAME, tTYPE)                           \
  tTYPE lrt_librcp_##sTYPENAME##_from_data(const uint8_t* data, size_t length) \
  {                                                                            \
    assert(length <= sizeof(tTYPE));                                           \
    memcpy(lrt_librcp_union_##sTYPENAME.bytes, data, length);                  \
    return lrt_librcp_union_##sTYPENAME.val;                                   \
  }                                                                            \
  const uint8_t* lrt_librcp_##sTYPENAME##_to_data(const tTYPE val)             \
  {                                                                            \
    lrt_librcp_union_##sTYPENAME.val = val;                                    \
    return lrt_librcp_union_##sTYPENAME.bytes;                                 \
  }                                                                            \
  void lrt_librcp_##sTYPENAME##_set_data(lrt_rbp_message_t* message,           \
                                         const tTYPE val)                      \
  {                                                                            \
    rcomm_message_insert_data(                                                 \
      message, lrt_librcp_##sTYPENAME##_to_data(val), sizeof(tTYPE), 0);       \
  }                                                                            \
  tTYPE lrt_librcp_##sTYPENAME##_get_data(lrt_rbp_message_t* message)          \
  {                                                                            \
    rcomm_message_read_data(                                                   \
      message, lrt_librcp_union_##sTYPENAME.bytes, sizeof(tTYPE), 0);          \
    return lrt_librcp_##sTYPENAME##_from_data(                                 \
      lrt_librcp_union_##sTYPENAME.bytes, sizeof(tTYPE));                      \
  }
#endif

LRT_LIBRCP_CONVERSION_IMPL(Bool, bool)
LRT_LIBRCP_CONVERSION_IMPL(Uint8, uint8_t)
LRT_LIBRCP_CONVERSION_IMPL(Int8, int8_t)
LRT_LIBRCP_CONVERSION_IMPL(Uint16, uint16_t)
LRT_LIBRCP_CONVERSION_IMPL(Int16, int16_t)
LRT_LIBRCP_CONVERSION_IMPL(Uint32, uint32_t)
LRT_LIBRCP_CONVERSION_IMPL(Int32, int32_t)
LRT_LIBRCP_CONVERSION_IMPL(Uint64, uint64_t)
LRT_LIBRCP_CONVERSION_IMPL(Int64, int64_t)
LRT_LIBRCP_CONVERSION_IMPL(Float, float)
LRT_LIBRCP_CONVERSION_IMPL(Double, double)

lrt_rcp_message_type_t
rcomm_get_litecomm_message_type(lrt_rbp_message_t* message);
void
rcomm_set_litecomm_message_type(lrt_rbp_message_t* message,
                                lrt_rcp_message_type_t type);

lrt_rcp_message_type_t
rcomm_get_litecomm_message_type(lrt_rbp_message_t* message)
{
  return (lrt_rcp_message_type_t)(message->data[4] & 0b11000000u);
}
void
rcomm_set_litecomm_message_type(lrt_rbp_message_t* message,
                                lrt_rcp_message_type_t type)
{
  message->data[4] = (message->data[4] & 0b00111111u) | type;
}
