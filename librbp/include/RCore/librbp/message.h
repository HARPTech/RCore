#ifndef LRT_LIBRBP_MESSAGE_H
#define LRT_LIBRBP_MESSAGE_H

#include "../../../../librcore/include/RCore/events.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /** @file */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define LRT_LIBRBP_BLOCK_START_BIT 0b10000000u
#define LRT_LIBRBP_MESSAGE_LIST_FREER(MSG) \
  lrt_rbp_message_free_internal(&MSG->data)
#define LRT_LIBRBP_MESSAGE_FREER(MSG) lrt_rbp_message_free(MSG)

  typedef uint8_t lrt_rbp_message_data_element;
  typedef uint8_t lrt_rbp_message_config_type;

  typedef enum lrt_rbp_message_config_t
  {
    LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8 = (1 << 0),
    LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC32 = (1 << 1)
  } lrt_rbp_message_config_t;

  typedef struct lrt_rbp_message_t
  {
    /**
     * @brief Data of the message.
     *
     * The data field contains the normal data of the message. When sending the
     * message, this gets converted into MIDI-like format (left-most bit
     * determines message start or end).
     */
    lrt_rbp_message_data_element* data;

    /** The length of the contained data. */
    size_t length;

    /** Available memory in the data array. Gets initialised at the start. */
    size_t _memory;

    lrt_rbp_message_config_type config;
  } lrt_rbp_message_t;

  lrt_rbp_message_t* lrt_rbp_message_create(size_t default_reserved_memory,
                                            lrt_rbp_message_config_type config);

  /** Initialise a message with a specified reserved memory and a config
   * field.
   */
  void lrt_rbp_message_init(lrt_rbp_message_t* message,
                            size_t default_reserved_memory,
                            lrt_rbp_message_config_type config);

  void lrt_rbp_message_free_internal(lrt_rbp_message_t* message);

  void lrt_rbp_message_free(lrt_rbp_message_t* message);

  lrt_rcore_event_t lrt_rbp_message_resize(lrt_rbp_message_t* message,
                                           size_t target_length);

  void lrt_rbp_message_copy(lrt_rbp_message_t* target,
                            const lrt_rbp_message_t* source);

  /** Get a configuration flag from the specified message struct. */
  bool lrt_rbp_message_check_config(const lrt_rbp_message_t* message,
                                    lrt_rbp_message_config_t config);

  /** Set a configuration flag into the specified message struct. */
  void lrt_rbp_message_set_config(lrt_rbp_message_t* message,
                                  lrt_rbp_message_config_t config,
                                  bool state);

  /** Calculate the minimum buffer length from a given message length. */
  size_t lrt_rbp_buffer_length_from_message_length(size_t msg_length);

  /** Calculate the minimum message length from a given buffer length. */
  size_t lrt_rbp_message_length_from_buffer_length(size_t buffer_length);

  void lrt_rbp_message_reset_data(lrt_rbp_message_t* message);

  /** Encode the message given in msg into the given buffer. Returns
   * LRT_RCORE_DATA_LEFT when it needs to be called again after sending the
   * current data chunk, LRT_RCORE_OK once encoding is finished. */
  lrt_rcore_event_t lrt_rbp_encode_message(lrt_rbp_message_t* msg,
                                           lrt_rbp_message_data_element* buffer,
                                           size_t buffer_length);

  /** Decode the message from the given buffer into the message msg. */
  lrt_rcore_event_t lrt_rbp_decode_message(
    lrt_rbp_message_t* msg,
    const lrt_rbp_message_data_element* buffer,
    size_t buffer_length);

  /** Validates the given buffer for correct encoding to be decoded by
   * lrt_rbp_decode_message.
   *
   * Validation is based on length and content of the buffer. */
  lrt_rcore_event_t lrt_rbp_is_block_valid(
    const lrt_rbp_message_data_element* buffer,
    size_t length);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
