#ifndef LRT_LIBRSP_STREAM_MSG_H
#define LRT_LIBRSP_STREAM_MSG_H

#include <RCore/librcp/message.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(                 \
  sPREFIX, sNAME, iDATA_INDEX, iDATA_POS)                        \
  bool sPREFIX##_is_##sNAME(sPREFIX##_block_t* block)            \
  {                                                              \
    return block->data[iDATA_INDEX] & iDATA_POS;                 \
  }                                                              \
  void sPREFIX##_set_##sNAME(sPREFIX##_block_t* block, bool val) \
  {                                                              \
    if(val)                                                      \
      block->data[iDATA_INDEX] |= iDATA_POS;                     \
    else                                                         \
      block->data[iDATA_INDEX] &= ~(iDATA_POS);                  \
  }

#include <assert.h>
#include <stdint.h>

#define LRT_LIBRSP_STREAM_MESSAGE(sPREFIX, iDATA_SIZE)                         \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, ack, 0, 0b01000000)         \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, reliable, 0, 0b00100000)    \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, sStart, 0, 0b00010000)      \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, sEnd, 0, 0b00001000)        \
  size_t sPREFIX##_get_data_size(sPREFIX##_block_t* block)                     \
  {                                                                            \
    return iDATA_SIZE - 3 - (sPREFIX##_is_reliable(block) ? 1 : 0);            \
  }                                                                            \
  void sPREFIX##_set_data(sPREFIX##_block_t* block, size_t pos, uint8_t val)   \
  {                                                                            \
    assert(pos < (iDATA_SIZE) - (sPREFIX##_is_reliable(block) ? 4 : 3));       \
    sPREFIX##_set_block_data(                                                  \
      block, pos + (sPREFIX##_is_reliable(block) ? 4 : 3), val);               \
  }                                                                            \
  uint8_t sPREFIX##_get_data(sPREFIX##_block_t* block, size_t pos)             \
  {                                                                            \
    assert(pos < (iDATA_SIZE) - (sPREFIX##_is_reliable(block) ? 4 : 3));       \
    return sPREFIX##_get_block_data(                                           \
      block, pos + (sPREFIX##_is_reliable(block) ? 4 : 3));                    \
  }                                                                            \
  bool sPREFIX##_is_iPacket(sPREFIX##_block_t* block)                          \
  {                                                                            \
    return !(sPREFIX##_is_sStart(block)) && !(sPREFIX##_is_sEnd(block));       \
  }                                                                            \
  bool sPREFIX##_is_tinyPacket(sPREFIX##_block_t* block)                       \
  {                                                                            \
    return sPREFIX##_is_sStart(block) && sPREFIX##_is_sEnd(block);             \
  }                                                                            \
  uint8_t sPREFIX##_get_sequence_number(sPREFIX##_block_t* block)              \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    return ((sPREFIX##_get_block_data(block, 0) & 0x0F) << 2) |                \
           ((sPREFIX##_get_block_data(block, 1) & 0b11000000) >> 6);           \
  }                                                                            \
  void sPREFIX##_set_sequence_number(sPREFIX##_block_t* block, uint8_t seq)    \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    assert(seq <= 0b00111111);                                                 \
    sPREFIX##_set_block_data(block,                                            \
                             0,                                                \
                             (sPREFIX##_get_block_data(block, 0) & 0xF0) |     \
                               ((seq & 0b00111100) >> 2));                     \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      1,                                                                       \
      (sPREFIX##_get_block_data(block, 1) & 0b00111111) |                      \
        ((seq & 0b00000011) << 6));                                            \
  }                                                                            \
  uint8_t sPREFIX##_get_next_sequence_number(sPREFIX##_block_t* block)         \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    return (sPREFIX##_get_block_data(block, 1) & 0b00111111);                  \
  }                                                                            \
  void sPREFIX##_set_next_sequence_number(sPREFIX##_block_t* block,            \
                                          uint8_t seq)                         \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    assert(seq <= 0b00111111);                                                 \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      1,                                                                       \
      (sPREFIX##_get_block_data(block, 1) & 0b11000000) | (seq & 0b00111111)); \
  }                                                                            \
  uint8_t sPREFIX##_get_litecomm_type(sPREFIX##_block_t* block)                \
  {                                                                            \
    return (                                                                   \
      (sPREFIX##_get_block_data(block, sPREFIX##_is_reliable(block) ? 2 : 1) & \
       0xF0) >>                                                                \
      4);                                                                      \
  }                                                                            \
  void sPREFIX##_set_litecomm_type(sPREFIX##_block_t* block, uint8_t type)     \
  {                                                                            \
    assert(type <= 0x0F);                                                      \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                      \
    sPREFIX##_set_block_data(block,                                            \
                             index,                                            \
                             (sPREFIX##_get_block_data(block, index) & 0x0F) | \
                               ((type & 0x0F) << 4));                          \
  }                                                                            \
  uint16_t sPREFIX##_get_litecomm_property(sPREFIX##_block_t* block)           \
  {                                                                            \
    uint16_t property = 0;                                                     \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    property = (sPREFIX##_get_block_data(block, index) & 0x0F);                \
    property <<= 8;                                                            \
    property |= sPREFIX##_get_block_data(block, index + 1);                    \
    return property;                                                           \
  }                                                                            \
  void sPREFIX##_set_litecomm_property(sPREFIX##_block_t* block, uint16_t val) \
  {                                                                            \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      index,                                                                   \
      ((sPREFIX##_get_block_data(block, index) & 0xF0)) |                      \
        ((val >> 8) & 0x0F));                                                  \
    sPREFIX##_set_block_data(block, index + 1, val & 0xFF);                    \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
