#ifndef LRT_LIBRCORE_CALLBACKS_H
#define LRT_LIBRCORE_CALLBACKS_H

#include "events.h"
#include "../../../librbp/include/RCore/librbp/message.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

  typedef lrt_rcore_event_t (*rcomm_transmit_data_cb)(const uint8_t* data,
                                                      void* userdata,
                                                      size_t bytes);
  typedef lrt_rcore_event_t (*rcomm_accept_block_cb)(lrt_rbp_message_t* message,
                                                     void* userdata);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
