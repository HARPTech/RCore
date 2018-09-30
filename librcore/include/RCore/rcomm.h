#ifndef LRT_LIBRCORE_RCOMM_H
#define LRT_LIBRCORE_RCOMM_H

#include "ack_stack.h"
#include "callbacks.h"
#include "events.h"
// For some reason, the sequence stack breaks the CPP of the message.h file.
// clang-format off
#include <RCore/librcp/message.h>
#include "sequence_stack.h"
#include "transmit_buffer.h"
#include <RCore/librbp/message.h>
#include <RCore/librsp/flags.h>
#include <RCore/librsp/litecomm.h>
#include <RCore/librsp/data.h>
#include <RCore/librcp/message.h>
// clang-format on
#include "../../../librbp/include/RCore/librbp/message.h"

#ifdef __cplusplus
extern "C"
{
#endif

  // Common aliases according to place of use.
  // typedef lrt_rbp_message_t rcomm_message_t;
  // typedef lrt_rbp_message_t rcomm_block_t;

#include <assert.h>

  /* @file
   * This file is the main interface generator for a RCore communication stack.
   * Use it as it is used in the test code.
   */

  typedef struct rcomm_handle_t
  {
    uint8_t* incoming_buffer;
    size_t incoming_buffer_size, maximum_incoming_buffer_size;

    uint8_t* outgoing_buffer;
    size_t outgoing_buffer_size;
    lrt_rbp_message_t incoming_message, outgoing_message;
    lrt_rcore_sequence_stack_t* sequence_stack;
    lrt_rcore_ack_stack_t* ack_stack;

    /* Callbacks */
    void* transmit_userdata;
    void* accept_userdata;
    rcomm_transmit_data_cb transmit;
    rcomm_accept_block_cb accept;
  } rcomm_handle_t;

  void rcomm_init(rcomm_handle_t* handle,
                  size_t message_default_reserved_memory,
                  size_t maximum_buffer_size,
                  size_t maximum_stack_size,
                  size_t maximum_queue_size,
                  lrt_rbp_message_config_type message_config);

  void rcomm_free(rcomm_handle_t* handle);

  rcomm_handle_t* rcomm_create(size_t message_default_reserved_memory,
                               size_t maximum_buffer_size,
                               size_t maximum_stack_size,
                               size_t maximum_queue_size,
                               lrt_rbp_message_config_type message_config);

  void rcomm_set_transmit_cb(rcomm_handle_t* handle,
                             rcomm_transmit_data_cb cb,
                             void* userdata);

  void rcomm_set_accept_cb(rcomm_handle_t* handle,
                           rcomm_accept_block_cb cb,
                           void* userdata);

  lrt_rcore_event_t rcomm_transmit_message(rcomm_handle_t* handle,
                                           lrt_rbp_message_t* message);

  lrt_rcore_event_t rcomm_send_message(rcomm_handle_t* handle,
                                       lrt_rbp_message_t* message);

  lrt_rcore_event_t rcomm_send_tb_entry(
    rcomm_handle_t* handle,
    lrt_rcore_transmit_buffer_entry_t* entry);

  lrt_rcore_event_t rcomm_transfer_message_to_tb(
    rcomm_handle_t* handle,
    lrt_rbp_message_t* message,
    lrt_rcore_transmit_buffer_t* tb);

  lrt_rcore_event_t rcomm_handle_complete_block(rcomm_handle_t* handle);

  lrt_rcore_event_t rcomm_parse_bytes(rcomm_handle_t* handle,
                                      const uint8_t* data,
                                      size_t length);

  lrt_rcore_event_t rcomm_send_ctrl(rcomm_handle_t* handle,
                                    uint8_t type,
                                    uint16_t property,
                                    lrt_rcp_message_type_t message_type,
                                    bool reliable);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
