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
  handle->ack_stack = lrt_rcore_ack_stack_init(
    maximum_stack_size / 2, maximum_stack_size, maximum_queue_size, 20);
  handle->sequence_stack =
    lrt_rcore_sequence_stack_init(maximum_stack_size, maximum_queue_size);
  handle->transmit_userdata = NULL;
  handle->accept_userdata = NULL;
  handle->accept = NULL;
  handle->transmit = NULL;

  handle->incoming_buffer_size = 0;
  handle->maximum_incoming_buffer_size = maximum_buffer_size;
  handle->outgoing_buffer_size = maximum_buffer_size;

  handle->incoming_buffer = calloc(sizeof(uint8_t), maximum_buffer_size);
  handle->outgoing_buffer = calloc(sizeof(uint8_t), maximum_buffer_size);
}

rcomm_handle_t*
rcomm_create(size_t message_default_reserved_memory,
             size_t maximum_buffer_size,
             size_t maximum_stack_size,
             size_t maximum_queue_size,
             lrt_rbp_message_config_type message_config)
{
  rcomm_handle_t* handle = (rcomm_handle_t*)calloc(1, sizeof(rcomm_handle_t));
  if(handle) {
    rcomm_init(handle,
               message_default_reserved_memory,
               maximum_buffer_size,
               maximum_stack_size,
               maximum_queue_size,
               message_config);
  }
  return handle;
}

void
rcomm_set_transmit_cb(rcomm_handle_t* handle,
                      rcomm_transmit_data_cb cb,
                      void* userdata)
{
  assert(handle != NULL);
  assert(cb != NULL);
  handle->transmit = cb;
  handle->transmit_userdata = userdata;
}

void
rcomm_set_accept_cb(rcomm_handle_t* handle,
                    rcomm_accept_block_cb cb,
                    void* userdata)
{
  assert(handle != NULL);
  assert(cb != NULL);
  handle->accept = cb;
  handle->accept_userdata = userdata;
}

lrt_rcore_event_t
rcomm_transmit_message(rcomm_handle_t* handle, const lrt_rbp_message_t* message)
{
  // Translate the message into outgoing data.
  lrt_rcore_event_t status = lrt_rbp_encode_message(
    message, handle->outgoing_buffer, handle->outgoing_buffer_size);

  // Transmit with the callback.
  status = handle->transmit(
    message->data,
    handle->transmit_userdata,
    lrt_rbp_buffer_length_from_message_length(message->length));
  return status;
}

lrt_rcore_event_t
rcomm_send_message(rcomm_handle_t* handle, const lrt_rbp_message_t* message)
{
  assert(handle != NULL);
  assert(message != NULL);
  assert(handle->transmit != NULL);
  lrt_rcore_event_t status = LRT_RCORE_OK;
  if(rcomm_message_has_flag(message, LRT_LIBRSP_RELIABLE)) {
    status = lrt_rcore_ack_stack_insert(handle->ack_stack, message);
    if(status != LRT_RCORE_OK) {
      return status;
    }
  }

  status = rcomm_transmit_message(handle, message);

  return status;
}

lrt_rcore_event_t
rcomm_send_tb_entry(rcomm_handle_t* handle,
                    lrt_rcore_transmit_buffer_entry_t* entry)
{
  lrt_rcore_event_t status = LRT_RCORE_OK;
  assert(handle != 0);
  assert(entry != 0);
  assert(entry->data != 0);
  assert(entry->data->l != 0);
  rcomm_message_set_flag(
    &handle->outgoing_message, LRT_LIBRSP_STREAM_START, true);
  rcomm_message_set_flag(
    &handle->outgoing_message, LRT_LIBRSP_RELIABLE, entry->reliable);
  rcomm_message_set_litecomm_type(&handle->outgoing_message, entry->type);
  rcomm_message_set_litecomm_property(&handle->outgoing_message,
                                      entry->property);
  rcomm_set_litecomm_message_type(&handle->outgoing_message,
                                  entry->message_type);
  while(status == LRT_RCORE_OK &&
        !lrt_rcore_transmit_buffer_entry_transmit_finished(
          entry)) { /* Each iteration handles a new block to be transmitted. */
    entry->transmit_offset = rcomm_message_insert_data(
      &handle->outgoing_message,
      (const uint8_t*)entry->data->s,
      entry->data->l,
      entry->transmit_offset); /* Set rest of block data fields. */
    rcomm_message_set_sequence_number(&handle->outgoing_message,
                                      entry->seq_number);
    rcomm_message_set_flag(
      &handle->outgoing_message,
      LRT_LIBRSP_STREAM_END,
      lrt_rcore_transmit_buffer_entry_transmit_finished(entry));
    status = rcomm_send_message(handle, &handle->outgoing_message);
    rcomm_message_set_flag(
      &handle->outgoing_message, LRT_LIBRSP_STREAM_START, false);
  }
  if(lrt_rcore_transmit_buffer_entry_transmit_finished(entry)) {
    lrt_rcore_transmit_buffer_free_send_slot(entry->origin, entry);
  }
  return status;
}

lrt_rcore_event_t
rcomm_transfer_block_to_tb(rcomm_handle_t* handle,
                           lrt_rbp_message_t* message,
                           lrt_rcore_transmit_buffer_t* tb)
{
  uint8_t packetTypeBits = rcomm_get_packetTypeBits(message);
  lrt_rcore_transmit_buffer_receive_data_bytes(
    tb,
    rcomm_message_get_litecomm_type(message),
    rcomm_message_get_litecomm_property(message),
    packetTypeBits,
    message->data + rcomm_message_get_data_offset(message),
    rcomm_message_get_data_size(message),
    rcomm_get_litecomm_message_type(message),
    rcomm_message_has_flag(message, LRT_LIBRSP_RELIABLE),
    rcomm_message_get_sequence_number(message));
  return LRT_RCORE_OK;
}
lrt_rcore_event_t
rcomm_handle_complete_block(rcomm_handle_t* handle)
{
  assert(handle->accept != NULL);

  if(!lrt_rbp_is_block_valid(handle->incoming_buffer,
                             handle->incoming_buffer_size)) {
    return LRT_RCORE_INVALID_BLOCK;
  }

  lrt_rcore_event_t status = LRT_RCORE_OK;

  // Decode message.
  status = lrt_rbp_decode_message(&handle->incoming_message,
                                  handle->incoming_buffer,
                                  handle->incoming_buffer_size);

  if(rcomm_message_has_flag(&handle->incoming_message, LRT_LIBRSP_ACK)) {
    return lrt_rcore_ack_stack_remove(handle->ack_stack,
                                      &handle->incoming_message);
  }
  if(rcomm_message_has_flag(&handle->incoming_message,
                            LRT_LIBRSP_STREAM_TINY)) {
    if(status == LRT_RCORE_OK) {
      status =
        handle->accept(&handle->incoming_message, handle->accept_userdata);
    }
    if(rcomm_message_has_flag(&handle->incoming_message, LRT_LIBRSP_RELIABLE)) {
      rcomm_message_set_flag(&handle->incoming_message, LRT_LIBRSP_ACK, true);
      handle->incoming_message.length = 7;
      if(status == LRT_RCORE_OK) {
        rcomm_transmit_message(handle, &handle->incoming_message);
      }
    }
  } else if(rcomm_message_has_flag(&handle->incoming_message,
                                   LRT_LIBRSP_RELIABLE)) {
    if(status == LRT_RCORE_OK) {
      status =
        lrt_rcore_sequence_stack_handle_message(handle->sequence_stack,
                                                &handle->incoming_message,
                                                handle->accept,
                                                handle->accept_userdata,
                                                handle);
    }
  } else { /* If the block is not reliable, it can just be given to the
            * acceptor.   \
            */
    if(status == LRT_RCORE_OK) {
      status =
        handle->accept(&handle->incoming_message, handle->accept_userdata);
    }
  }
  return status;
}
lrt_rcore_event_t
rcomm_parse_bytes(rcomm_handle_t* handle, const uint8_t* data, size_t length)
{
  lrt_rcore_event_t status = LRT_RCORE_OK;
  for(size_t i = 0; i < length; ++i) {
    if(data[i] &
       LRT_LIBRBP_BLOCK_START_BIT) { /* Begin of new block, old block can be
                                        given to next handler. */
      if(handle->incoming_buffer_size > 0 &&
         handle->incoming_buffer_size % 8 ==
           0) { /* Only if the byte counter is dividable by 8, the block would
                 * \
                 * be discarded otherwise. This makes small blocks possible
                 * for      \ faster transmissions of small values. */
        if(status == LRT_RCORE_OK) {
          status = rcomm_handle_complete_block(handle);
        }
      }
      handle->incoming_buffer_size = 0;
    } else if(handle->incoming_buffer_size ==
              handle->maximum_incoming_buffer_size - 1) {
      if(status == LRT_RCORE_OK) {
        status = rcomm_handle_complete_block(handle);
      }
      handle->incoming_buffer_size = 0;
    }
    /* This seems like an overlong message. It should
     * still be able to     \
     * be parsed, if receiving is stopped now. This
     * behaviour can be       \ useful in
     * environments like SPI or with harmful inputs.
     */
    /* Write the received byte into the block. */

    assert(handle->incoming_buffer_size < handle->maximum_incoming_buffer_size);
    assert(i < length);
    handle->incoming_buffer[handle->incoming_buffer_size++] = data[i];
  } /* Also handle finished blocks if there are no additional bytes. */
  if(handle->incoming_buffer_size > 0 &&
     handle->incoming_buffer_size % 8 == 0) {
    if(status == LRT_RCORE_OK) {
      status = rcomm_handle_complete_block(handle);
    }
    handle->incoming_buffer_size = 0;
  }
  return status;
}
lrt_rcore_event_t
rcomm_send_ctrl(rcomm_handle_t* handle,
                uint8_t type,
                uint16_t property,
                lrt_rcp_message_type_t message_type,
                bool reliable)
{
  lrt_rcore_event_t status = LRT_RCORE_OK;
  assert(handle != 0);
  rcomm_message_set_flag(
    &handle->outgoing_message, LRT_LIBRSP_STREAM_TINY, true);
  rcomm_message_set_flag(
    &handle->outgoing_message, LRT_LIBRSP_RELIABLE, reliable);
  rcomm_message_set_litecomm_type(&handle->outgoing_message, type);
  rcomm_message_set_litecomm_property(&handle->outgoing_message, property);
  rcomm_set_litecomm_message_type(&handle->outgoing_message, message_type);
  rcomm_message_set_sequence_number(&handle->outgoing_message, 0);
  status = rcomm_send_message(handle, &handle->outgoing_message);
  return status;
}

void
rcomm_free(rcomm_handle_t* handle)
{
  lrt_rbp_message_free_internal(&handle->incoming_message);
  lrt_rbp_message_free_internal(&handle->outgoing_message);

  free(handle->incoming_buffer);
  free(handle->outgoing_buffer);

  lrt_rcore_ack_stack_free(handle->ack_stack);
  lrt_rcore_sequence_stack_free(handle->sequence_stack);

  free(handle);
}
