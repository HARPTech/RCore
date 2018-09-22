#ifndef LRT_LIBRSP_DATA_H
#define LRT_LIBRSP_DATA_H

#include "../../../../librbp/include/RCore/librbp/message.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>

  inline size_t lrt_size_t_min(size_t a, size_t b)
  {
    if(a > b)
      return b;
    return a;
  }

  /**
   * @brief Return the offset of encapsulated data in this message.
   */
  inline size_t rcomm_message_get_data_offset(const lrt_rbp_message_t* message)
  {
    return 5;
  }

  /**
   * Return the maximum size of the data segment in this message.
   */
  inline size_t rcomm_message_get_data_size(const lrt_rbp_message_t* message)
  {
    size_t size = message->_memory - rcomm_message_get_data_offset(message);
    if(lrt_rbp_message_check_config(message,
                                    LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8)) {
      size -= 1;
    }
    if(lrt_rbp_message_check_config(message,
                                    LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC32)) {
      size -= 4;
    }
    return size;
  }

  inline size_t rcomm_message_insert_data(lrt_rbp_message_t* message,
                                          const uint8_t* data,
                                          size_t length,
                                          size_t offset)
  {
    assert(message != NULL);
    assert(message->data != NULL);
    assert(message->_memory >= 7);

    /* Try to encode as much of the given data as possible and return how much \
     * is left. This function completely disregards any other message data and \
     * only handles the inner message data. */
    size_t size = lrt_size_t_min(rcomm_message_get_data_size(message), length);

    memcpy(message->data + rcomm_message_get_data_offset(message),
           data + offset,
           size);
    // The new message length is equal to the amount of bytes copied.
    message->length = size;

    return size + offset;
  }

  /** Reads data from a message. The data can be longer than the message content
   * and can span across multiple messages.
   */
  inline size_t rcomm_message_read_data(const lrt_rbp_message_t* message,
                                        uint8_t* target,
                                        size_t target_length,
                                        size_t offset)
  {
    size_t size =
      lrt_size_t_min(rcomm_message_get_data_size(message), target_length);

    memcpy(target + offset,
           message->data + rcomm_message_get_data_offset(message),
           size);

    return size + offset;
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
