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
  inline size_t lrt_size_t_max(size_t a, size_t b)
  {
    if(a > b)
      return a;
    return b;
  }

  /**
   * @brief Return the offset of encapsulated data in this message.
   */
  size_t rcomm_message_get_data_offset(const lrt_rbp_message_t* message);

  size_t rcomm_message_get_trailing_size(const lrt_rbp_message_t* message);

  /**
   * Return the maximum size of the data segment in this message.
   */
  size_t rcomm_message_get_data_size(const lrt_rbp_message_t* message);

  size_t rcomm_message_get_trailing_offset(const lrt_rbp_message_t* message);

  size_t rcomm_message_insert_data(lrt_rbp_message_t* message,
                                   const uint8_t* data,
                                   size_t length,
                                   size_t offset);

  /** Reads data from a message. The data can be longer than the message content
   * and can span across multiple messages.
   */
  size_t rcomm_message_read_data(const lrt_rbp_message_t* message,
                                 uint8_t* target,
                                 size_t target_length,
                                 size_t offset);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
