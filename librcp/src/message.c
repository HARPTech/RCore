#include "../include/RCore/librcp/message.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef BIG_ENDIAN
#define LRT_LIBRCP_CONVERSION_IMPL(sTYPENAME, tTYPE)                           \
  tTYPE lrt_librcp_##sTYPENAME##_from_data(const uint8_t* data, size_t length) \
  {                                                                            \
    assert(length <= sizeof(tTYPE));                                           \
    memcpy(lrt_librcp_union_##sTYPENAME.bytes, data, length);                  \
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
  }
#else
#define LRT_LIBRCP_CONVERSION_IMPL(sTYPENAME, tTYPE)                           \
  union lrt_librcp_##sTYPENAME##_t                                             \
  {                                                                            \
    tTYPE val;                                                                 \
    uint8_t bytes[sizeof(tTYPE)];                                              \
  };                                                                           \
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
