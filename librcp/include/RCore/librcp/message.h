#ifndef LRT_LIBRCP_STREAM_MSG_H
#define LRT_LIBRCP_STREAM_MSG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#ifdef BIG_ENDIAN
#define LRT_LIBRCP_TYPES_SETTER(sPREFIX, sTYPENAME, tTYPE)               \
  int16_t sPREFIX##_set_data_##sTYPENAME(                                \
    sPREFIX##_block_t* block, tTYPE data, int16_t counter)               \
  {                                                                      \
    if(counter < 0) {                                                    \
      counter = sizeof(tTYPE);                                           \
    }                                                                    \
    size_t i = 0;                                                        \
    for(; counter - i > 0 && i < sPREFIX##_get_data_size(block);         \
        ++i, --counter) {                                                \
      size_t offset = (sizeof(tTYPE) - 1) - (counter - 1) * 8;           \
      sPREFIX##_set_data(                                                \
        block, i, ((data & (((tTYPE)0xFF) << offset)) >> offset) & 0xFF) \
    }                                                                    \
    return counter;                                                      \
  }                                                                      \
  _Pragma("error")

#define LRT_LIBRCP_TYPES(sPREFIX)                             \
  int16_t sPREFIX##_set_data_Uint8(                           \
    sPREFIX##_block_t* block, uint8_t data, int16_t counter)  \
  {                                                           \
    sPREFIX##_set_data(block, 0, data);                       \
    return 0;                                                 \
  }                                                           \
  int16_t sPREFIX##_set_data_Uint16(                          \
    sPREFIX##_block_t* block, uint16_t data, int16_t counter) \
  {                                                           \
    sPREFIX##_set_data(block, 0, data & 0x00FF);              \
    sPREFIX##_set_data(block, 1, (data & 0xFF00) >> 8);       \
    return 0;                                                 \
  }                                                           \
  int16_t sPREFIX##_set_data_Int8(                            \
    sPREFIX##_block_t* block, int8_t data, int16_t counter)   \
  {                                                           \
    sPREFIX##_set_data(block, 0, data);                       \
    return 0;                                                 \
  }                                                           \
  int16_t sPREFIX##_set_data_Int16(                           \
    sPREFIX##_block_t* block, int16_t data, int16_t counter)  \
  {                                                           \
    sPREFIX##_set_data(block, 0, data & 0x00FF);              \
    sPREFIX##_set_data(block, 1, (data & 0xFF00) >> 8);       \
    return 0;                                                 \
  }

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
    for(size_t i = 0; counter > 0 && i < sPREFIX##_get_data_size(block); \
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
    } else {                                                             \
      sPREFIX##__union_##tTYPE.data = *data;                             \
    }                                                                    \
    for(size_t i = 0; counter > 0 && i < sPREFIX##_get_data_size(block); \
        ++i, --counter) {                                                \
      sPREFIX##__union_##tTYPE.bytes[counter - 1] =                      \
        sPREFIX##_get_data(block, i);                                    \
    }                                                                    \
    *data = sPREFIX##__union_##tTYPE.data;                               \
    return counter;                                                      \
  }

#define LRT_LIBRCP_TYPES(sPREFIX)                      \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Bool, bool)       \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint8, uint8_t)   \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int8, int8_t)     \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint16, uint16_t) \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int16, int16_t)   \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint32, uint32_t) \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int32, int32_t)   \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Uint64, uint64_t) \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Int64, int64_t)   \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Float, float)     \
  LRT_LIBRCP_TYPES_FOR_TYPE(sPREFIX, Double, double)

#endif

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
