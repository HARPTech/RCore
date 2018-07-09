#ifndef LRT_LIBRSP_STREAM_MSG_H
#define LRT_LIBRSP_STREAM_MSG_H

#include <RCore/librcp/message.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>
#include <stdint.h>

#define LRT_LIBRSP_STREAM_MESSAGE(sPREFIX, iDATA_SIZE)                       \
  void sPREFIX##_set_data(sPREFIX##_block_t* block, size_t pos, uint8_t val) \
  {                                                                          \
    assert(pos < (iDATA_SIZE)-4);                                            \
    sPREFIX##_set_block_data(block, pos + 4, val);                           \
  }                                                                          \
  uint8_t sPREFIX##_get_data(sPREFIX##_block_t* block, size_t pos)           \
  {                                                                          \
    assert(pos < (iDATA_SIZE)-4);                                            \
    return sPREFIX##_get_block_data(block, pos + 4);                         \
  }                                                                          \
  bool sPREFIX##_is_ack(sPREFIX##_block_t* block)                            \
  {                                                                          \
    return sPREFIX##_get_block_data(block, 0) & 0b10000000;                  \
  }                                                                          \
  void sPREFIX##_set_ack(sPREFIX##_block_t* block, bool ack)                 \
  {                                                                          \
    sPREFIX##_set_block_data(                                                \
      block,                                                                 \
      0,                                                                     \
      (sPREFIX##_get_block_data(block, 0) & 0b01111111) | ack ? 0b10000000   \
                                                              : 0);          \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
