#ifndef LRT_LIBRBP_BLOCK_H
#define LRT_LIBRBP_BLOCK_H

#include <RCore/librsp/stream_message.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LRT_LIBRBP_BLOCK_START_BIT 0b10000000u

#ifdef LRT_RCORE_DEBUG
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_MEMBERS(sPREFIX, iBLOCK_SIZE) \
  char out_str[(iBLOCK_SIZE / 8) * (iBLOCK_SIZE * 8 + 1 + 8)];
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_FUNCTIONS(sPREFIX, iBLOCK_SIZE) \
  inline const char* sPREFIX##_to_str(sPREFIX##_block_t* block)       \
  {                                                                   \
    for(size_t i = 0; i < block->significant_bytes; ++i) {            \
      for(size_t j = 0; j < 9u; ++j) {                                \
        if(j < 8)                                                     \
          block->out_str[i * 9u + j] =                                \
            block->data[i] & 1u << (7u - j) ? '1' : '0';              \
        else                                                          \
          block->out_str[i * 9u + j] = ' ';                           \
      }                                                               \
    }                                                                 \
    block->out_str[iBLOCK_SIZE * 9u] = '\0';                          \
    return block->out_str;                                            \
  }                                                                   \
  const char* sPREFIX##_to_str(sPREFIX##_block_t* block);
#else
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_MEMBERS(sPREFIX, iBLOCK_SIZE)
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_FUNCTIONS(sPREFIX, iBLOCK_SIZE) \
  inline const char* sPREFIX##_to_str(sPREFIX##_block_t* block)       \
  {                                                                   \
    return "\0";                                                      \
  }                                                                   \
  const char* sPREFIX##_to_str(sPREFIX##_block_t* block);
#endif

#define LRT_LIBRBP_BLOCK_STRUCT(sPREFIX, iBLOCK_SIZE, INTERNAL)             \
  typedef struct sPREFIX##_block_t                                          \
  {                                                                         \
    uint8_t data[iBLOCK_SIZE];                                              \
    size_t significant_bytes;                                               \
    LRT_LIBRBP_BLOCK_STRUCT_DEBUG_MEMBERS(sPREFIX, iBLOCK_SIZE)             \
  } sPREFIX##_block_t;                                                      \
                                                                            \
  inline void sPREFIX##_init_block(sPREFIX##_block_t* block)                \
  {                                                                         \
    assert(iBLOCK_SIZE % 8 == 0);                                           \
    for(size_t i = 0; i < iBLOCK_SIZE; ++i) {                               \
      block->data[i] = (i == 0) ? 0b10000000 : 0;                           \
    }                                                                       \
    block->significant_bytes = iBLOCK_SIZE;                                 \
  }                                                                         \
  inline uint8_t sPREFIX##_get_block_data(sPREFIX##_block_t* block,         \
                                          size_t pos)                       \
  {                                                                         \
    assert(pos < iBLOCK_SIZE - 1);                                          \
    return ((block->data[pos + (pos / 7U)] & (0xFFU >> ((pos % 7U) + 1U)))  \
            << ((pos % 7U) + 1U)) |                                         \
           (((block->data[pos + 1U + (pos / 7U)] &                          \
              (0xFFU << (7U - ((pos % 7U) + 1U)))) &                        \
             0b01111111U) >>                                                \
            (7U - ((pos % 7U) + 1U)));                                      \
  }                                                                         \
  inline void sPREFIX##_set_block_data(                                     \
    sPREFIX##_block_t* block, size_t pos, uint8_t val)                      \
  {                                                                         \
    assert(pos < iBLOCK_SIZE - 1u);                                         \
    block->data[pos + (pos / 7u)] =                                         \
      (block->data[pos + (pos / 7u)] &                                      \
       (0xFFu << ((size_t)(8u - ((pos % 7u) + 1u))))) |                     \
      ((uint8_t)(val >> ((pos % 7u) + 1u)));                                \
    block->data[pos + 1U + (pos / 7U)] =                                    \
      (block->data[pos + 1U + (pos / 7U)] & (0xFFU >> ((pos % 7U) + 2U))) | \
      ((uint8_t)(val << (7u - ((pos % 7u) + 1u))) & 0b01111111u);           \
  }                                                                         \
  inline bool sPREFIX##_is_block_valid(sPREFIX##_block_t* block)            \
  {                                                                         \
    if((block->data[0] & LRT_LIBRBP_BLOCK_START_BIT) == 0) {                \
      return false;                                                         \
    }                                                                       \
    for(size_t i = 1; i < iBLOCK_SIZE; ++i) {                               \
      if(block->data[i] & LRT_LIBRBP_BLOCK_START_BIT) {                     \
        return false;                                                       \
      }                                                                     \
    }                                                                       \
    return true;                                                            \
  }                                                                         \
                                                                            \
  void sPREFIX##_init_block(sPREFIX##_block_t* block);                      \
  uint8_t sPREFIX##_get_block_data(sPREFIX##_block_t* block, size_t pos);   \
  void sPREFIX##_set_block_data(                                            \
    sPREFIX##_block_t* block, size_t pos, uint8_t val);                     \
  bool sPREFIX##_is_block_valid(sPREFIX##_block_t* block);                  \
  LRT_LIBRBP_BLOCK_STRUCT_DEBUG_FUNCTIONS(sPREFIX, iBLOCK_SIZE)             \
  INTERNAL(sPREFIX, (iBLOCK_SIZE - (iBLOCK_SIZE / 8u)))

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
