#ifndef LRT_LIBRCORE_EVENTS_H
#define LRT_LIBRCORE_EVENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum lrt_rcore_event_t
  {
    LRT_RCORE_OK = 0,
    LRT_RCORE_STACK_FULL = 1,
    LRT_RCORE_ACK_STACK_FULL = 2,
    LRT_RCORE_STACK_DEPTH_EXHAUSTED = 3,
    LRT_RCORE_NO_ACK_ENTRY_FOUND = 4,
    LRT_RCORE_MAXIMUM_SUBSCRIPTIONS = 5,
    LRT_RCORE_SUBSCRIPTION_MAP_PUT_ERROR = 6
  } lrt_rcore_event_t;

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
