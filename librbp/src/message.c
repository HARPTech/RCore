#include "../include/RCore/librbp/message.h"
#include "../include/RCore/librbp/internal/crc.h"
#include <stdlib.h>

size_t
lrt_rbp_buffer_length_from_message_length(size_t msg_length)
{
  return (((msg_length - 1) / 7u) + 1u) * 8u;
}

size_t
lrt_rbp_message_length_from_buffer_length(size_t buffer_length)
{
  return (buffer_length / 8u) * 7u;
}

bool
lrt_rbp_message_check_config(const lrt_rbp_message_t* message,
                             lrt_rbp_message_config_t config)
{
  assert(message != NULL);
  return (message->config & config) == config;
}

void
lrt_rbp_message_set_config(lrt_rbp_message_t* message,
                           lrt_rbp_message_config_t config,
                           bool state)
{
  assert(message != NULL);
  if(state) {
    message->config |= config;
  } else {
    message->config &= ~config;
  }
}

void
lrt_rbp_message_reset_data(lrt_rbp_message_t* message)
{
  if(message->data != NULL) {
    memset(message->data,
           sizeof(lrt_rbp_message_data_element) * message->_memory,
           message->_memory);
  }
}

void
lrt_rbp_message_copy(lrt_rbp_message_t* target, const lrt_rbp_message_t* source)
{
  assert(target != NULL);
  assert(source != NULL);
  assert(source->data != NULL);

  if(target->_memory < source->length) {
    lrt_rbp_message_resize(target, source->length);
  }

  assert(target->_memory >= source->length);

  target->config = source->config;
  target->length = source->length;
  memcpy(target->data, source->data, source->length);
}

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
  // Transform the target length to a desired value.
  target_length = lrt_rbp_message_length_from_buffer_length(
    lrt_rbp_buffer_length_from_message_length(target_length - 1));

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
    message->length = target_length;
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
lrt_rbp_encode_message(lrt_rbp_message_t* msg,
                       uint8_t* buffer,
                       size_t buffer_length)
{
  assert(buffer_length >= 2);

  // Resize the message to the correct size before sending.
  lrt_rbp_message_resize(msg, msg->length);

  // Set the CRC checksum.
  lrt_rcore_event_t status = lrt_rbp_set_crc(msg);
  if(status != LRT_RCORE_OK) {
    return status;
  }

  size_t i = 0;

  // Prepare the buffer for sending.
  buffer[0] = LRT_LIBRBP_BLOCK_START_BIT;
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

  lrt_rcore_event_t status = lrt_rbp_validate_crc(msg);

  return status;
}

lrt_rcore_event_t
lrt_rbp_is_block_valid(const uint8_t* buffer, size_t length)
{
  if(length < 8) {
    return LRT_RCORE_BLOCK_TOO_SHORT;
  }
  // The length has to be calculated with a +1 because arrays start at 0.
  if(length % 8 != 0) {
    return LRT_RCORE_BLOCK_NOT_DIVIDABLE_BY_8;
  }
  if((buffer[0] & LRT_LIBRBP_BLOCK_START_BIT) == 0) {
    return LRT_RCORE_BLOCK_NO_START_BIT;
  }
  for(size_t i = 1; i < length; ++i) {
    if((buffer[i] & LRT_LIBRBP_BLOCK_START_BIT) != 0) {
      return LRT_RCORE_BLOCK_START_BIT_INSIDE_MESSAGE;
    }
  }
  return LRT_RCORE_OK;
}
