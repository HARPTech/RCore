#ifndef LRT_LIBRCORE_EVENTS_H
#define LRT_LIBRCORE_EVENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum lrt_rcore_event_t
  {
    LRT_RCORE_OK = 0,
    LRT_RCORE_INVALID_BLOCK = 1,
    LRT_RCORE_STACK_FULL = 2,
    LRT_RCORE_ACK_STACK_MAP_FULL = 3,
    LRT_RCORE_ACK_STACK_QUEUE_FULL = 4,
    LRT_RCORE_STACK_DEPTH_EXHAUSTED = 5,
    LRT_RCORE_NO_ACK_ENTRY_FOUND = 6,
    LRT_RCORE_MAXIMUM_SUBSCRIPTIONS = 7,
    LRT_RCORE_SUBSCRIPTION_MAP_PUT_ERROR = 8,
    LRT_RCORE_TRANSMIT_ERROR = 9,
    LRT_RCORE_DATA_LEFT = 10,
    LRT_RCORE_ALLOC_FAILED = 11,
  } lrt_rcore_event_t;

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
