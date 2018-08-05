#ifndef LRT_LIBRSP_STREAM_MESSAGE_H
#define LRT_LIBRSP_STREAM_MESSAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <byteswap.h>

#if BIG_ENDIAN == 1
#define LRT_LIBRSP_BYTESWAP16_IF_NEEDED(VALUE) VALUE = __bswap_16(VALUE)
#define LRT_LIBRSP_BYTESWAP32_IF_NEEDED(VALUE) VALUE = __bswap_32(VALUE)
#else
#define LRT_LIBRSP_BYTESWAP16_IF_NEEDED(VALUE)
#define LRT_LIBRSP_BYTESWAP32_IF_NEEDED(VALUE)
#endif

#define LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(                        \
  sPREFIX, sNAME, iDATA_INDEX, iDATA_POS)                               \
  inline bool sPREFIX##_is_##sNAME(sPREFIX##_block_t* block)            \
  {                                                                     \
    return block->data[iDATA_INDEX] & iDATA_POS;                        \
  }                                                                     \
  inline void sPREFIX##_set_##sNAME(sPREFIX##_block_t* block, bool val) \
  {                                                                     \
    if(val)                                                             \
      block->data[iDATA_INDEX] |= iDATA_POS;                            \
    else                                                                \
      block->data[iDATA_INDEX] &= ~(iDATA_POS);                         \
  }

#include <assert.h>
#include <stdint.h>

#define LRT_LIBRSP_ACK 0b01000000u
#define LRT_LIBRSP_RELIABLE 0b00100000u
#define LRT_LIBRSP_STREAM_START 0b00010000u
#define LRT_LIBRSP_STREAM_END 0b00001000u
#define LRT_LIBRSP_STREAM_TINY 0b00011000u
#define LRT_LIBRSP_STREAM_RUNNING 0b00000000u

#define LRT_LIBRSP_STREAM_MESSAGE(sPREFIX, iDATA_SIZE)                         \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(sPREFIX, ack, 0u, LRT_LIBRSP_ACK)    \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(                                     \
    sPREFIX, reliable, 0u, LRT_LIBRSP_RELIABLE)                                \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(                                     \
    sPREFIX, sStart, 0u, LRT_LIBRSP_STREAM_START)                              \
  LRT_LIBRSP_STREAM_MESSAGE_BIT_FUNCTIONS(                                     \
    sPREFIX, sEnd, 0u, LRT_LIBRSP_STREAM_END)                                  \
  inline uint8_t sPREFIX##_get_packetTypeBits(sPREFIX##_block_t* block)        \
  {                                                                            \
    return block->data[0] & 0b01111000u;                                       \
  }                                                                            \
  inline uint8_t sPREFIX##_get_packetStreamBits(sPREFIX##_block_t* block)      \
  {                                                                            \
    return block->data[0] & 0b00011000u;                                       \
  }                                                                            \
  inline size_t sPREFIX##_get_data_size(sPREFIX##_block_t* block)              \
  {                                                                            \
    return iDATA_SIZE - 3 - (sPREFIX##_is_reliable(block) ? 1 : 0);            \
  }                                                                            \
  inline void sPREFIX##_set_data(                                              \
    sPREFIX##_block_t* block, size_t pos, uint8_t val)                         \
  {                                                                            \
    assert(pos < (iDATA_SIZE) - (sPREFIX##_is_reliable(block) ? 4 : 3));       \
    sPREFIX##_set_block_data(                                                  \
      block, pos + (sPREFIX##_is_reliable(block) ? 4 : 3), val);               \
  }                                                                            \
  inline uint8_t sPREFIX##_get_data(sPREFIX##_block_t* block, size_t pos)      \
  {                                                                            \
    assert(pos < (iDATA_SIZE) - (sPREFIX##_is_reliable(block) ? 4 : 3));       \
    return sPREFIX##_get_block_data(                                           \
      block, pos + (sPREFIX##_is_reliable(block) ? 4 : 3));                    \
  }                                                                            \
  inline bool sPREFIX##_is_iPacket(sPREFIX##_block_t* block)                   \
  {                                                                            \
    return !(sPREFIX##_is_sStart(block)) && !(sPREFIX##_is_sEnd(block));       \
  }                                                                            \
  inline bool sPREFIX##_is_tinyPacket(sPREFIX##_block_t* block)                \
  {                                                                            \
    return sPREFIX##_is_sStart(block) && sPREFIX##_is_sEnd(block);             \
  }                                                                            \
  inline uint8_t sPREFIX##_get_sequence_number(sPREFIX##_block_t* block)       \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    return ((sPREFIX##_get_block_data(block, 0) & 0x0Fu) << 2u) |              \
           ((sPREFIX##_get_block_data(block, 1) & 0b11000000u) >> 6u);         \
  }                                                                            \
  inline void sPREFIX##_set_sequence_number(sPREFIX##_block_t* block,          \
                                            uint8_t seq)                       \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
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
  inline uint8_t sPREFIX##_get_next_sequence_number(sPREFIX##_block_t* block)  \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    return (sPREFIX##_get_block_data(block, 1) & 0b00111111u);                 \
  }                                                                            \
  inline void sPREFIX##_set_next_sequence_number(sPREFIX##_block_t* block,     \
                                                 uint8_t seq)                  \
  {                                                                            \
    assert(sPREFIX##_is_reliable(block));                                      \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      1,                                                                       \
      (sPREFIX##_get_block_data(block, 1) & 0b11000000u) |                     \
        (seq & 0b00111111u));                                                  \
  }                                                                            \
  inline uint8_t sPREFIX##_get_litecomm_type(sPREFIX##_block_t* block)         \
  {                                                                            \
    return ((sPREFIX##_get_block_data(                                         \
               block, sPREFIX##_is_reliable(block) ? 2u : 1u) &                \
             0xF0u) >>                                                         \
            4u);                                                               \
  }                                                                            \
  inline void sPREFIX##_set_litecomm_type(sPREFIX##_block_t* block,            \
                                          uint8_t type)                        \
  {                                                                            \
    assert(type <= 0x0F);                                                      \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      index,                                                                   \
      (sPREFIX##_get_block_data(block, index) & 0x0Fu) |                       \
        ((type & 0x0Fu) << 4u));                                               \
  }                                                                            \
  inline uint16_t sPREFIX##_get_litecomm_property(sPREFIX##_block_t* block)    \
  {                                                                            \
    uint16_t property = 0;                                                     \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    property = (sPREFIX##_get_block_data(block, index) & 0x0Fu);               \
    property <<= 8u;                                                           \
    property |= sPREFIX##_get_block_data(block, index + 1u);                   \
    LRT_LIBRSP_BYTESWAP16_IF_NEEDED(property);                                 \
    return property;                                                           \
  }                                                                            \
  inline void sPREFIX##_set_litecomm_property(sPREFIX##_block_t* block,        \
                                              uint16_t val)                    \
  {                                                                            \
    LRT_LIBRSP_BYTESWAP16_IF_NEEDED(val);                                      \
    size_t index = sPREFIX##_is_reliable(block) ? 2 : 1;                       \
    sPREFIX##_set_block_data(                                                  \
      block,                                                                   \
      index,                                                                   \
      ((sPREFIX##_get_block_data(block, index) & 0xF0u)) |                     \
        ((uint8_t)(val >> 8u) & 0x0Fu));                                       \
    sPREFIX##_set_block_data(block, index + 1, val & 0xFFu);                   \
  }                                                                            \
  inline size_t sPREFIX##_insert_data(sPREFIX##_block_t* block,                \
                                      const uint8_t* data,                     \
                                      size_t length,                           \
                                      size_t offset)                           \
  {                                                                            \
    /* Try to encode as much of the given data as possible and return how much \
     * is left. This function completely disregards any other message data and \
     * only handles the inner message data. */                                 \
    for(size_t i = 0;                                                          \
        length - offset - 1 > 0 && i < sPREFIX##_get_data_size(block);         \
        ++i, ++offset) {                                                       \
      sPREFIX##_set_data(block, i, data[offset]);                              \
    }                                                                          \
    if(length < sPREFIX##_get_data_size(block)) {                              \
      block->significant_bytes = (length % 8) * 8;                             \
    }                                                                          \
    return offset;                                                             \
  }                                                                            \
  inline void sPREFIX##_read_data(                                             \
    sPREFIX##_block_t* block, uint8_t* target, size_t length)                  \
  {                                                                            \
    for(size_t i = 0; i < iDATA_SIZE && i < length; ++i)                       \
      target[i] = sPREFIX##_get_data(block, i);                                \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
