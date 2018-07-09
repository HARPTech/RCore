#ifndef LRT_LIBRBP_STREAM_MSG_H
#define LRT_LIBRBP_STREAM_MSG_H

#include <RCore/librsp/stream_message.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef LRT_RCORE_DEBUG
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_MEMBERS(sPREFIX, iBLOCK_SIZE) \
  char out_str[iBLOCK_SIZE * 8 + 1 + 8];
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_FUNCTIONS(sPREFIX, iBLOCK_SIZE) \
  const char* sPREFIX##_to_str(sPREFIX##_block_t* block)              \
  {                                                                   \
    for(size_t i = 0; i < iBLOCK_SIZE; ++i) {                         \
      for(size_t j = 0; j < 9; ++j) {                                 \
        if(j < 8)                                                     \
          block->out_str[i * 9 + j] =                                 \
            block->data[i] & 1 << (7 - j) ? '1' : '0';                \
        else                                                          \
          block->out_str[i * 9 + j] = ' ';                            \
      }                                                               \
    }                                                                 \
    block->out_str[iBLOCK_SIZE * 9] = '\0';                           \
    return block->out_str;                                            \
  }
#else
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_MEMBERS(sPREFIX, iBLOCK_SIZE)
#define LRT_LIBRBP_BLOCK_STRUCT_DEBUG_FUNCTIONS(sPREFIX, iBLOCK_SIZE) \
  const char* sPREFIX##_to_str(sPREFIX##_block_t* block) { return '\0'; }
#endif

#define LRT_LIBRBP_BLOCK_STRUCT(sPREFIX, iBLOCK_SIZE, INTERNAL)               \
  typedef struct sPREFIX##_block_t                                            \
  {                                                                           \
    uint8_t data[iBLOCK_SIZE];                                                \
    LRT_LIBRBP_BLOCK_STRUCT_DEBUG_MEMBERS(sPREFIX, iBLOCK_SIZE)               \
  } sPREFIX##_block_t;                                                        \
                                                                              \
  void sPREFIX##_init(sPREFIX##_block_t* block)                               \
  {                                                                           \
    for(size_t i = 0; i < iBLOCK_SIZE; ++i) {                                 \
      block->data[i] = (i == 0) ? 0b10000000 : 0;                             \
    }                                                                         \
  }                                                                           \
  uint8_t sPREFIX##_get_block_data(sPREFIX##_block_t* block, size_t pos)      \
  {                                                                           \
    assert(pos < iBLOCK_SIZE - 1);                                            \
    return ((block->data[pos] & (0xFF >> ((pos % 7) + 1)))                    \
            << ((pos % 7) + 1)) |                                             \
           (((block->data[pos + 1] & (0xFF << (7 - ((pos % 7) + 1)))) &       \
             0b01111111) >>                                                   \
            (7 - ((pos % 7) + 1)));                                           \
  }                                                                           \
  void sPREFIX##_set_block_data(                                              \
    sPREFIX##_block_t* block, size_t pos, uint8_t val)                        \
  {                                                                           \
    assert(pos < iBLOCK_SIZE - 1);                                            \
    block->data[pos] = (block->data[pos] & (0xFF << (8 - ((pos % 7) + 1)))) | \
                       (val >> ((pos % 7) + 1));                              \
    block->data[pos + 1] =                                                    \
      (block->data[pos + 1] & (0xFF >> ((pos % 7) + 1))) |                    \
      ((val << (7 - ((pos % 7) + 1))) & 0b01111111);                          \
  }                                                                           \
  void sPREFIX##_get_block_bit(sPREFIX##_block_t* block, size_t bit)          \
  {                                                                           \
    assert(bit * 8 < iBLOCK_SIZE - 1);                                        \
    return sPREFIX##_get_block_data(block, bit / 8) & 1 << 8 - (bit % 8);     \
  }                                                                           \
  void sPREFIX##_set_block_bit(sPREFIX##_block_t* block, size_t bit, bool val) \
  {                                                                           \
    sPREFIX##_set_block_data(                                                 \
      block,                                                                  \
      bit / 8,                                                                      \
      (sPREFIX##_get_block_data(block, 0) & (~(8 - (bit % 8)))) | ack ? 0b10000000 \
                                                              : 0);           \
  }
  INTERNAL(sPREFIX, iBLOCK_SIZE)
  LRT_LIBRSP_STREAM_MESSAGE(sPREFIX, (iBLOCK_SIZE - 1))

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
