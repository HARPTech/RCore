#ifndef LRT_LIBRCORE_CALLBACKS_H
#define LRT_LIBRCORE_CALLBACKS_H

#include "events.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define LRT_RCORE_CALLBACKS(sPREFIX)                       \
  typedef lrt_rcore_event_t (*sPREFIX##_transmit_data_cb)( \
    const uint8_t* data, void* userdata, size_t bytes);    \
  typedef lrt_rcore_event_t (*sPREFIX##_accept_block_cb)(  \
    sPREFIX##_block_t * block, void* userdata);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
