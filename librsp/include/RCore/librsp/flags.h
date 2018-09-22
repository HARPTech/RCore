#ifndef LRT_LIBRSP_FLAGS_H
#define LRT_LIBRSP_FLAGS_H

/** @file */

#include "../../../../librbp/include/RCore/librbp/message.h"
#include "../../../../librcore/include/RCore/events.h"
#include "byte_swap.h"

#ifdef __cplusplus
extern "C"
{
#endif

  enum lrt_rsp_flag
  {
    LRT_LIBRSP_ACK = 0b10000000u,
    LRT_LIBRSP_RELIABLE = 0b01000000,
    LRT_LIBRSP_STREAM_START = 0b00100000,
    LRT_LIBRSP_STREAM_END = 0b00010000,
    LRT_LIBRSP_STREAM_TINY = 0b00110000,
  };

  inline uint8_t rcomm_get_packetTypeBits(const lrt_rbp_message_t* message)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 1);
    return message->data[0] & 0b00110000;
  }

  inline bool rcomm_message_has_flag(const lrt_rbp_message_t* message,
                                     enum lrt_rsp_flag flag)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 1);
    return (message->data[0] & flag) == flag;
  }

  inline void rcomm_message_set_flag(lrt_rbp_message_t* message,
                                     enum lrt_rsp_flag flag,
                                     bool state)
  {
    assert(message->data != NULL);
    assert(message->_memory >= 1);
    if(state) {
      message->data[0] |= flag;
    } else {
      message->data[0] &= ~flag;
    }
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
