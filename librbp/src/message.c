#include "../include/RCore/librbp/message.h"
#include <stdlib.h>

size_t
lrt_rbp_buffer_length_from_message_length(size_t msg_length);
size_t
lrt_rbp_message_length_from_buffer_length(size_t buffer_length);
bool
lrt_rbp_message_check_config(const lrt_rbp_message_t* message,
                             lrt_rbp_message_config_t config);

void
lrt_rbp_message_copy(lrt_rbp_message_t* target,
                     const lrt_rbp_message_t* source);

lrt_rbp_message_t*
lrt_rbp_message_create(size_t default_reserved_memory,
                       lrt_rbp_message_config_type config)
{
  lrt_rbp_message_t* message = malloc(sizeof(lrt_rbp_message_t));
  lrt_rbp_message_init(message, default_reserved_memory, config);
  return message;
}

void
lrt_rbp_message_init(lrt_rbp_message_t* message,
                     size_t default_reserved_memory,
                     lrt_rbp_message_config_type config)
{
  message->length = 0;
  message->_memory = 14;
  message->data = calloc(sizeof(uint8_t), default_reserved_memory);
  message->config = config;
}

void
lrt_rbp_message_free_internal(lrt_rbp_message_t* message)
{
  assert(message != NULL);
  if(message->data != NULL) {
    free(message->data);
  }
}

void
lrt_rbp_message_free(lrt_rbp_message_t* message)
{
  assert(message != NULL);

  lrt_rbp_message_free_internal(message);

  free(message);
}

lrt_rcore_event_t
lrt_rbp_message_resize(lrt_rbp_message_t* message, size_t target_length)
{
  if(target_length <= message->_memory) {
    message->length = target_length;
    return LRT_RCORE_OK;
  }

  // Allocate a new buffer if this has not been done before.
  if(message->data == NULL) {
    message->data = calloc(sizeof(lrt_rbp_message_data_element), target_length);

    if(message->data == NULL) {
      return LRT_RCORE_ALLOC_FAILED;
    }

    message->_memory = target_length;
    return LRT_RCORE_OK;
  }

  // Resize the buffer.
  lrt_rbp_message_data_element* data = realloc(
    message->data, sizeof(lrt_rbp_message_data_element) * target_length);
  if(data == NULL) {
    return LRT_RCORE_ALLOC_FAILED;
  }
  message->data = data;
  message->_memory = target_length;
  message->length = target_length;
  return LRT_RCORE_OK;
}

lrt_rcore_event_t
lrt_rbp_encode_message(const lrt_rbp_message_t* msg,
                       uint8_t* buffer,
                       size_t buffer_length)
{
  assert(buffer_length >= 2);

  size_t i = 0;

  // Prepare the buffer for sending.
  buffer[0] = 0b10000000;
  for(i = 1; i < buffer_length; ++i) {
    buffer[i] = 0;
  }

  // Fill buffer with provided data.
  for(i = 0; i < buffer_length && i < msg->length; ++i) {
    buffer[i + (i / 7u)] =
      (uint8_t)(buffer[i + (i / 7u)] &
                (0xFFu << ((size_t)(8u - ((i % 7u) + 1u))))) |
      ((uint8_t)(msg->data[i] >> ((i % 7u) + 1u)));
    buffer[i + 1U + (i / 7U)] =
      (buffer[i + 1U + (i / 7U)] & (0xFFU >> ((i % 7U) + 2U))) |
      ((uint8_t)(msg->data[i] << (7u - ((i % 7u) + 1u))) & 0b01111111u);
  }

  return LRT_RCORE_OK;
}

lrt_rcore_event_t
lrt_rbp_decode_message(lrt_rbp_message_t* msg,
                       const uint8_t* buffer,
                       size_t buffer_length)
{
  assert(msg != NULL);

  msg->length = lrt_rbp_message_length_from_buffer_length(buffer_length);

  lrt_rbp_message_resize(msg, msg->length);

  for(size_t i = 0; i < msg->length; ++i) {
    msg->data[i] =
      (uint8_t)((uint8_t)((buffer[i + (i / 7U)] & (0xFFU >> ((i % 7U) + 1U)))
                          << ((i % 7U) + 1U)) |
                (uint8_t)(((buffer[i + 1U + (i / 7U)] &
                            (0xFFU << (7U - ((i % 7U) + 1U)))) &
                           0b01111111U) >>
                          (7U - ((i % 7U) + 1U))));
  }

  return LRT_RCORE_OK;
}

bool
lrt_rbp_is_block_valid(const uint8_t* buffer, size_t length)
{
  if(length < 8) {
    return false;
  }
  if(length % 8 != 0) {
    return false;
  }
  if((buffer[0] & LRT_LIBRBP_BLOCK_START_BIT) == 0) {
    return false;
  }
  for(size_t i = 1; i < length; ++i) {
    if((buffer[i] & LRT_LIBRBP_BLOCK_START_BIT) != 0) {
      return false;
    }
  }
  return true;
}
