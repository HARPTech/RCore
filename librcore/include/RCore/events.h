#ifndef LRT_LIBRCORE_EVENTS_H
#define LRT_LIBRCORE_EVENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum lrt_rcore_event_t
  {
    LRT_RCORE_OK,
    LRT_RCORE_INVALID_BLOCK,
    LRT_RCORE_STACK_FULL,
    LRT_RCORE_ACK_STACK_MAP_FULL,
    LRT_RCORE_ACK_STACK_QUEUE_FULL,
    LRT_RCORE_NO_ACK_ENTRY_FOUND,
    LRT_RCORE_STACK_DEPTH_EXHAUSTED,
    LRT_RCORE_MAXIMUM_SUBSCRIPTIONS,
    LRT_RCORE_SUBSCRIPTION_MAP_PUT_ERROR,
    LRT_RCORE_TRANSMIT_ERROR,
    LRT_RCORE_DATA_LEFT,
    LRT_RCORE_ALLOC_FAILED,
    LRT_RCORE_INTERNAL_SEQUENCE_STACK_ERROR,
    LRT_RCORE_GENERIC_ACCEPTOR_ERROR,
    LRT_RCORE_GENERIC_TRANSMITTER_ERROR,
    LRT_RCORE_NOT_ACCEPTED,
    LRT_RCORE_NOT_SUBSCRIBED,
    LRT_RCORE_INVALID_TRANSMIT_BUFFER_ENTRY,
    LRT_RCORE_CRC_MISMATCH,

    LRT_RCORE_BLOCK_TOO_SHORT,
    LRT_RCORE_BLOCK_NOT_DIVIDABLE_BY_8,
    LRT_RCORE_BLOCK_NO_START_BIT,
    LRT_RCORE_BLOCK_START_BIT_INSIDE_MESSAGE,

    LRT_RCORE_EVENT_COUNT
  } lrt_rcore_event_t;

  extern const char* lrt_rcore_event_names[LRT_RCORE_EVENT_COUNT];

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#if defined(__cplusplus) && !defined(DISABLE_ADVANCED_CPP)
// Automatic conversion into string for enum when using C++.
#include <iostream>
inline std::ostream&
operator<<(std::ostream& os, const lrt_rcore_event_t& e)
{
  os << "RCore Event " << static_cast<size_t>(e) << ": "
     << lrt_rcore_event_names[static_cast<size_t>(e)];
  return os;
}
#endif

#endif
