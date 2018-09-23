#ifndef LRT_LIBRCORE_ACK_STACK_H
#define LRT_LIBRCORE_ACK_STACK_H

#include "../../../librbp/include/RCore/librbp/message.h"
#include "callbacks.h"
#include "events.h"
#include "internal/hashtable.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct lrt_rcore_ack_stack_t lrt_rcore_ack_stack_t;
  typedef struct rcomm_handle_t rcomm_handle_t;

  lrt_rcore_ack_stack_t* lrt_rcore_ack_stack_init(
    size_t stack_size,
    size_t maximum_stack_size,
    size_t maximum_queue_size,
    size_t ack_ns_avg_sampling_rate);

  void lrt_rcore_ack_stack_free(lrt_rcore_ack_stack_t* stack);

  size_t lrt_rcore_ack_stack_count_pending(const lrt_rcore_ack_stack_t* stack);

  lrt_rcore_event_t lrt_rcore_ack_stack_insert(
    lrt_rcore_ack_stack_t* stack,
    const lrt_rbp_message_t* message);

  lrt_rcore_event_t lrt_rcore_ack_stack_remove(
    lrt_rcore_ack_stack_t* stack,
    const lrt_rbp_message_t* message);

  lrt_rcore_event_t lrt_rcore_ack_stack_tick(lrt_rcore_ack_stack_t* stack,
                                             uint32_t ns_since_last_tick,
                                             rcomm_handle_t* handle);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
