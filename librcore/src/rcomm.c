#include "../include/RCore/rcomm.h"

void
rcomm_init(rcomm_handle_t* handle,
           size_t message_default_reserved_memory,
           size_t maximum_buffer_size,
           size_t maximum_stack_size,
           size_t maximum_queue_size,
           lrt_rbp_message_config_type message_config)
{
  lrt_rbp_message_init(
    &handle->incoming_message, message_default_reserved_memory, message_config);
  lrt_rbp_message_init(
    &handle->outgoing_message, message_default_reserved_memory, message_config);
  lrt_rcore_ack_stack_init(maximum_stack_size, maximum_queue_size);
  lrt_rcomm_init_sequence_stack(&handle->sequence_stack);
  handle->transmit_userdata = NULL;
  handle->accept_userdata = NULL;
  handle->accept = NULL;
  handle->transmit = NULL;
  handle->byte_counter = 0;

  handle->incoming_buffer_size = maximum_buffer_size;
  handle->maximum_incoming_buffer_size = maximum_buffer_size;
  handle->outgoing_buffer_size = maximum_buffer_size;

  handle->incoming_buffer = calloc(sizeof(uint8_t), maximum_buffer_size);
  handle->outgoing_buffer = calloc(sizeof(uint8_t), maximum_buffer_size);
}
