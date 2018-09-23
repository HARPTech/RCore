#ifndef LRT_LIBRCORE_SEQUENCE_STACK_H
#define LRT_LIBRCORE_SEQUENCE_STACK_H

#include "callbacks.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>

  typedef struct lrt_rcore_sequence_stack_t lrt_rcore_sequence_stack_t;
  typedef struct rcomm_handle_t rcomm_handle_t;

  lrt_rcore_sequence_stack_t* lrt_rcore_sequence_stack_init(
    size_t maximum_entries_in_use,
    size_t maximum_queue_size);

  void lrt_rcore_sequence_stack_free(lrt_rcore_sequence_stack_t* stack);

  lrt_rcore_event_t lrt_rcore_sequence_stack_handle_message(
    lrt_rcore_sequence_stack_t* stack,
    lrt_rbp_message_t* message,
    rcomm_accept_block_cb acceptor,
    void* acceptor_userdata,
    rcomm_handle_t* handle);

  size_t lrt_rcore_sequence_stack_get_entries_in_use(
    lrt_rcore_sequence_stack_t* stack);
  size_t lrt_rcore_sequence_stack_get_messages_in_use(
    lrt_rcore_sequence_stack_t* stack);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
