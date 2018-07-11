#ifndef LRT_LIBRSP_STREAM_MESSAGE_H
#define LRT_LIBRSP_STREAM_MESSAGE_H

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

#define LRT_LIBRSP_STREAM_START 0b00010000u
#define LRT_LIBRSP_STREAM_END 0b00001000u
#define LRT_LIBRSP_STREAM_TINY 0b00011000u
#define LRT_LIBRSP_STREAM_RUNNING 0b00000000u

#define LRT_LIBRSP_STREAM_MESSAGE(sPREFIX, iDATA_SIZE)                         \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, ack, 0u, 0b01000000u)       \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, reliable, 0u, 0b00100000u)  \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, sStart, 0u, 0b00010000u)    \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, sEnd, 0u, 0b00001000u)      \
  uint8_t sPREFIX##_get_packetTypeBits(sPREFIX##_block_t* block)               \
  {                                                                            \
    return block->data[0] & 0b01111000u;                                       \
  }                                                                            \
  uint8_t sPREFIX##_get_packetStreamBits(sPREFIX##_block_t* block)             \
  {                                                                            \
    return block->data[0] & 0b00011000u;                                       \
  }                                                                            \
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
    return ((sPREFIX##_get_block_data(block, 0) & 0x0Fu) << 2u) |              \
           ((sPREFIX##_get_block_data(block, 1) & 0b11000000u) >> 6u);         \
  }                                                                            \
  void sPREFIX##_set_sequence_number(sPREFIX##_block_t* block, uint8_t seq)    \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    assert(seq <= 0b00111111);                                                 \
    sPREFIX##_set_block_data(block,                                            \
                             0,                                                \
                             (sPREFIX##_get_block_data(block, 0) & 0xF0u) |    \
                               ((seq & 0b00111100u) >> 2u));                   \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      1,                                                                       \
      (sPREFIX##_get_block_data(block, 1) & 0b00111111u) |                     \
        ((seq & 0b00000011u) << 6u));                                          \
  }                                                                            \
  uint8_t sPREFIX##_get_next_sequence_number(sPREFIX##_block_t* block)         \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    return (sPREFIX##_get_block_data(block, 1) & 0b00111111u);                 \
  }                                                                            \
  void sPREFIX##_set_next_sequence_number(sPREFIX##_block_t* block,            \
                                          uint8_t seq)                         \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    assert(seq <= 0b00111111u);                                                \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      1,                                                                       \
      (sPREFIX##_get_block_data(block, 1) & 0b11000000u) |                     \
        (seq & 0b00111111u));                                                  \
  }                                                                            \
  uint8_t sPREFIX##_get_litecomm_type(sPREFIX##_block_t* block)                \
  {                                                                            \
    return ((sPREFIX##_get_block_data(                                         \
               block, sPREFIX##_is_reliable(block) ? 2u : 1u) &                \
             0xF0u) >>                                                         \
            4u);                                                               \
  }                                                                            \
  void sPREFIX##_set_litecomm_type(sPREFIX##_block_t* block, uint8_t type)     \
  {                                                                            \
    assert(type <= 0x0F);                                                      \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      index,                                                                   \
      (sPREFIX##_get_block_data(block, index) & 0x0Fu) |                       \
        ((type & 0x0Fu) << 4u));                                               \
  }                                                                            \
  uint16_t sPREFIX##_get_litecomm_property(sPREFIX##_block_t* block)           \
  {                                                                            \
    uint16_t property = 0;                                                     \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    property = (sPREFIX##_get_block_data(block, index) & 0x0Fu);               \
    property <<= 8u;                                                           \
    property |= sPREFIX##_get_block_data(block, index + 1u);                   \
    return property;                                                           \
  }                                                                            \
  void sPREFIX##_set_litecomm_property(sPREFIX##_block_t* block, uint16_t val) \
  {                                                                            \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      index,                                                                   \
      ((sPREFIX##_get_block_data(block, index) & 0xF0u)) |                     \
        ((uint8_t)(val >> 8u) & 0x0Fu));                                       \
    sPREFIX##_set_block_data(block, index + 1, val & 0xFFu);                   \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
