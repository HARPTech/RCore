#ifndef LRT_LIBRSP_LITECOMM_H
#define LRT_LIBRSP_LITECOMM_H

#include "../../../../librbp/include/RCore/librbp/message.h"
#include "byte_swap.h"

#ifdef __cplusplus
extern "C"
{
#endif

  inline uint16_t rcomm_message_get_sequence_number(
    const lrt_rbp_message_t* message)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 2);
    uint16_t seq = 0;
    seq = message->data[0] & 0b00001111u;
    seq <<= 8u;
    seq |= message->data[1];
    LRT_LIBRSP_BYTESWAP16_IF_NEEDED(seq);
    return seq;
  }

  inline void rcomm_message_set_sequence_number(lrt_rbp_message_t* message,
                                                uint16_t seq)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 2);
    message->data[0] = (message->data[0] & 0xF0u) | (seq & 0x0F00u);
    message->data[1] = seq & 0x00FFu;
  }

  inline uint8_t rcomm_message_get_litecomm_type(
    const lrt_rbp_message_t* message)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 3);
    return (message->data[2] & 0xF0u) >> 4u;
  }

  inline void rcomm_message_set_litecomm_type(lrt_rbp_message_t* message,
                                              uint8_t type)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 3);
    message->data[2] = (message->data[2] & 0x0Fu) | ((type & 0x0Fu) << 4u);
  }

  inline uint16_t rcomm_message_get_litecomm_property(
    const lrt_rbp_message_t* message)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 3);
    uint16_t property = 0;
    property = message->data[2] & 0x0Fu;
    property <<= 8u;
    property |= message->data[3];
    LRT_LIBRSP_BYTESWAP16_IF_NEEDED(property);
    return property;
  }
  inline void rcomm_message_set_litecomm_property(lrt_rbp_message_t* message,
                                                  uint16_t property)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 3);
    message->data[2] =
      (message->data[2] & 0xF0u) | ((property & 0x0F00u) >> 8u);
    message->data[3] = property & 0xFF;
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
