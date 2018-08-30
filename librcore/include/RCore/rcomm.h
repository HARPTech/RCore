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
// clang-format on
#include <RCore/librbp/block.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>

  /* @file
   * This file is the main interface generator for a RCore communication stack.
   * Use it as it is used in the test code.
   */

#define LRT_RCORE_RCOMM_DEFINE_PROTOCOL_DEFINITIONS(                           \
  sPREFIX, iBLOCK_SIZE, tMESSAGE, iSTACK_WIDTH, iSTACK_DEPTH, iACK_STACK_SIZE) \
  LRT_LIBRBP_BLOCK_STRUCT(sPREFIX, iBLOCK_SIZE, tMESSAGE)                      \
  LRT_RCORE_CALLBACKS(sPREFIX)                                                 \
  LRT_RCORE_ACK_STACK_DEFINITIONS(sPREFIX, iBLOCK_SIZE, iACK_STACK_SIZE)       \
  LRT_LIBRCP_TYPES_DEFINITIONS(sPREFIX)                                        \
  typedef struct sPREFIX##_handle_t                                            \
  {                                                                            \
  } sPREFIX##_handle_t;                                                        \
  void sPREFIX##_init(sPREFIX##_handle_t* handle);                             \
  sPREFIX##_handle_t* sPREFIX##_create();                                      \
  void sPREFIX##_free(sPREFIX##_handle_t* handle);                             \
  void sPREFIX##_set_transmit_cb(sPREFIX##_handle_t* handle,                   \
                                 sPREFIX##_transmit_data_cb cb,                \
                                 void* userdata);                              \
  void sPREFIX##_set_accept_cb(                                                \
    sPREFIX##_handle_t* handle, sPREFIX##_accept_block_cb cb, void* userdata); \
  lrt_rcore_event_t sPREFIX##_send_block(                                      \
    sPREFIX##_handle_t* handle, sPREFIX##_block_t* block, size_t bytes);       \
  lrt_rcore_event_t sPREFIX##_send_tb_entry(                                   \
    sPREFIX##_handle_t* handle, lrt_rcore_transmit_buffer_entry_t* entry);     \
  lrt_rcore_event_t sPREFIX##_transfer_block_to_tb(                            \
    sPREFIX##_handle_t* handle,                                                \
    sPREFIX##_block_t* block,                                                  \
    lrt_rcore_transmit_buffer_t* tb);                                          \
  lrt_rcore_event_t sPREFIX##_handle_complete_block(                           \
    sPREFIX##_handle_t* handle);                                               \
  lrt_rcore_event_t sPREFIX##_parse_bytes(                                     \
    sPREFIX##_handle_t* handle, const uint8_t* data, size_t length);

#define LRT_RCORE_RCOMM_DEFINE_PROTOCOL(                                       \
  sPREFIX, iBLOCK_SIZE, tMESSAGE, iSTACK_WIDTH, iSTACK_DEPTH, iACK_STACK_SIZE) \
  LRT_LIBRBP_BLOCK_STRUCT(sPREFIX, iBLOCK_SIZE, tMESSAGE)                      \
  LRT_RCORE_CALLBACKS(sPREFIX)                                                 \
  LRT_RCORE_SEQUENCE_STACK(sPREFIX, iBLOCK_SIZE, iSTACK_WIDTH, iSTACK_DEPTH)   \
  LRT_RCORE_ACK_STACK(sPREFIX, iBLOCK_SIZE, iACK_STACK_SIZE)                   \
  LRT_LIBRCP_TYPES(sPREFIX)                                                    \
  typedef struct sPREFIX##_handle_t                                            \
  {                                                                            \
    sPREFIX##_block_t block, outgoing_block;                                   \
    size_t byte_counter;                                                       \
    sPREFIX##_sequence_stack_t sequence_stack;                                 \
    sPREFIX##_ack_stack_t ack_stack;                                           \
    /* Callbacks */                                                            \
    void* transmit_userdata;                                                   \
    void* accept_userdata;                                                     \
    sPREFIX##_transmit_data_cb transmit;                                       \
    sPREFIX##_accept_block_cb accept;                                          \
  } sPREFIX##_handle_t;                                                        \
  void sPREFIX##_init(sPREFIX##_handle_t* handle)                              \
  {                                                                            \
    sPREFIX##_init_block(&handle->block);                                      \
    sPREFIX##_init_block(&handle->outgoing_block);                             \
    sPREFIX##_init_ack_stack(&handle->ack_stack);                              \
    sPREFIX##_init_sequence_stack(&handle->sequence_stack);                    \
    handle->transmit_userdata = NULL;                                          \
    handle->accept_userdata = NULL;                                            \
    handle->accept = NULL;                                                     \
    handle->transmit = NULL;                                                   \
    handle->byte_counter = 0;                                                  \
  }                                                                            \
  sPREFIX##_handle_t* sPREFIX##_create()                                       \
  {                                                                            \
    sPREFIX##_handle_t* handle =                                               \
      (sPREFIX##_handle_t*)calloc(1, sizeof(sPREFIX##_handle_t));              \
    if(handle) {                                                               \
      sPREFIX##_init(handle);                                                  \
    }                                                                          \
    return handle;                                                             \
  }                                                                            \
  void sPREFIX##_free(sPREFIX##_handle_t* handle)                              \
  {                                                                            \
    assert(handle != NULL);                                                    \
    free(handle);                                                              \
  }                                                                            \
  void sPREFIX##_set_transmit_cb(                                              \
    sPREFIX##_handle_t* handle, sPREFIX##_transmit_data_cb cb, void* userdata) \
  {                                                                            \
    assert(handle != NULL);                                                    \
    assert(cb != NULL);                                                        \
    handle->transmit = cb;                                                     \
    handle->transmit_userdata = userdata;                                      \
  }                                                                            \
  void sPREFIX##_set_accept_cb(                                                \
    sPREFIX##_handle_t* handle, sPREFIX##_accept_block_cb cb, void* userdata)  \
  {                                                                            \
    assert(handle != NULL);                                                    \
    assert(cb != NULL);                                                        \
    handle->accept = cb;                                                       \
    handle->accept_userdata = userdata;                                        \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_send_block(                                      \
    sPREFIX##_handle_t* handle, sPREFIX##_block_t* block, size_t bytes)        \
  {                                                                            \
    assert(handle != NULL);                                                    \
    assert(block != NULL);                                                     \
    assert(bytes % 8 == 0);                                                    \
    assert(bytes <= iBLOCK_SIZE);                                              \
    assert(handle->transmit != NULL);                                          \
    lrt_rcore_event_t status = LRT_RCORE_OK;                                   \
    if(sPREFIX##_is_reliable(block)) {                                         \
      status = sPREFIX##_ack_stack_insert(&handle->ack_stack, block);          \
      if(status != LRT_RCORE_OK) {                                             \
        return status;                                                         \
      }                                                                        \
    }                                                                          \
    if(bytes <= 0) {                                                           \
      bytes = iBLOCK_SIZE;                                                     \
    }                                                                          \
    status = handle->transmit(block->data, handle->transmit_userdata, bytes);  \
    return status;                                                             \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_send_tb_entry(                                   \
    sPREFIX##_handle_t* handle, lrt_rcore_transmit_buffer_entry_t* entry)      \
  {                                                                            \
    lrt_rcore_event_t status = LRT_RCORE_OK;                                   \
    assert(handle != 0);                                                       \
    assert(entry != 0);                                                        \
    assert(entry->data != 0);                                                  \
    assert(entry->data->l != 0);                                               \
    sPREFIX##_set_sStart(&handle->outgoing_block, true);                       \
    sPREFIX##_set_reliable(&handle->outgoing_block, entry->reliable);          \
    sPREFIX##_set_litecomm_type(&handle->outgoing_block, entry->type);         \
    sPREFIX##_set_litecomm_property(&handle->outgoing_block, entry->property); \
    sPREFIX##_set_litecomm_message_type(&handle->outgoing_block,               \
                                        entry->message_type);                  \
    while(status == LRT_RCORE_OK &&                                            \
          !lrt_rcore_transmit_buffer_entry_transmit_finished(entry)) {         \
      /* Each iteration handles a new block to be transmitted. */              \
      entry->transmit_offset =                                                 \
        sPREFIX##_insert_data(&handle->outgoing_block,                         \
                              (const uint8_t*)entry->data->s,                  \
                              entry->data->l,                                  \
                              entry->transmit_offset);                         \
      /* Set rest of block data fields. */                                     \
      sPREFIX##_set_sequence_number(&handle->outgoing_block,                   \
                                    entry->seq_number);                        \
      sPREFIX##_set_sEnd(                                                      \
        &handle->outgoing_block,                                               \
        lrt_rcore_transmit_buffer_entry_transmit_finished(entry));             \
      status = sPREFIX##_send_block(handle,                                    \
                                    &handle->outgoing_block,                   \
                                    handle->outgoing_block.significant_bytes); \
      sPREFIX##_set_sStart(&handle->outgoing_block, false);                    \
    }                                                                          \
    if(lrt_rcore_transmit_buffer_entry_transmit_finished(entry)) {             \
      lrt_rcore_transmit_buffer_free_send_slot(entry->origin, entry);          \
    }                                                                          \
    return status;                                                             \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_transfer_block_to_tb(                            \
    sPREFIX##_handle_t* handle,                                                \
    sPREFIX##_block_t* block,                                                  \
    lrt_rcore_transmit_buffer_t* tb)                                           \
  {                                                                            \
    uint8_t packetTypeBits = sPREFIX##_get_packetTypeBits(block);              \
    for(size_t i = 1; i - 1 < sPREFIX##_get_data_size(block) - 1; ++i) {       \
      lrt_rcore_transmit_buffer_receive_data_byte(                             \
        tb,                                                                    \
        sPREFIX##_get_litecomm_type(block),                                    \
        sPREFIX##_get_litecomm_property(block),                                \
        packetTypeBits& LRT_LIBRSP_STREAM_START,                               \
        sPREFIX##_get_data(block, i),                                          \
        sPREFIX##_get_litecomm_message_type(block),                            \
        sPREFIX##_is_reliable(block),                                          \
        sPREFIX##_get_sequence_number(block));                                 \
    }                                                                          \
    /* The last byte gets information about the state of sEnd. */              \
    lrt_rcore_transmit_buffer_receive_data_byte(                               \
      tb,                                                                      \
      sPREFIX##_get_litecomm_type(block),                                      \
      sPREFIX##_get_litecomm_property(block),                                  \
      packetTypeBits,                                                          \
      sPREFIX##_get_data(block, sPREFIX##_get_data_size(block) - 1),           \
      sPREFIX##_get_litecomm_message_type(block),                              \
      sPREFIX##_is_reliable(block),                                            \
      sPREFIX##_get_sequence_number(block));                                   \
    return LRT_RCORE_OK;                                                       \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_handle_complete_block(                           \
    sPREFIX##_handle_t* handle)                                                \
  {                                                                            \
    assert(handle->accept != NULL);                                            \
    if(!sPREFIX##_is_block_valid(&handle->block)) {                            \
      return LRT_RCORE_INVALID_BLOCK;                                          \
    }                                                                          \
    lrt_rcore_event_t status = LRT_RCORE_OK;                                   \
    /* Blocks have to be handled according to the configuration of this        \
     * RComm stack. They are handled over the sequence stack. */               \
    if(sPREFIX##_is_ack(&handle->block)) {                                     \
      /* This block is just an ACKnowledge block. Mark the internal            \
       * ACK-Stack, so this block does not get sent again. */                  \
      return sPREFIX##_ack_stack_remove(&handle->ack_stack, &handle->block);   \
    }                                                                          \
    if(sPREFIX##_is_tinyPacket(&handle->block)) {                              \
      /* Can be directly given to the acceptor. */                             \
      if(status == LRT_RCORE_OK) {                                             \
        status = handle->accept(&handle->block, handle->accept_userdata);      \
      }                                                                        \
      if(sPREFIX##_is_reliable(&handle->block)) {                              \
        /* ACKnowledge the packet by changing the ACK bit and only sending 8   \
         * bytes. */                                                           \
        sPREFIX##_set_ack(&handle->block, true);                               \
        if(status == LRT_RCORE_OK) {                                           \
          handle->transmit(handle->block.data, handle->transmit_userdata, 8u); \
        }                                                                      \
      }                                                                        \
    } else if(sPREFIX##_is_reliable(&handle->block)) {                         \
      if(status == LRT_RCORE_OK) {                                             \
        status =                                                               \
          sPREFIX##_sequence_stack_handle_block(&handle->sequence_stack,       \
                                                &handle->block,                \
                                                handle->accept,                \
                                                handle->accept_userdata,       \
                                                handle->transmit,              \
                                                handle->transmit_userdata);    \
      }                                                                        \
    } else {                                                                   \
      /* If the block is not reliable, it can just be given to the acceptor.   \
       */                                                                      \
      if(status == LRT_RCORE_OK) {                                             \
        status = handle->accept(&handle->block, handle->accept_userdata);      \
      }                                                                        \
    }                                                                          \
    return status;                                                             \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_parse_bytes(                                     \
    sPREFIX##_handle_t* handle, const uint8_t* data, size_t length)            \
  {                                                                            \
    lrt_rcore_event_t status = LRT_RCORE_OK;                                   \
    for(size_t i = 0; i < length; ++i) {                                       \
      if(data[i] & LRT_LIBRBP_BLOCK_START_BIT) {                               \
        /* Begin of new block, old block can be given to next handler. */      \
        if(handle->byte_counter > 0 && handle->byte_counter % 8 == 0) {        \
          /* Only if the byte counter is dividable by 8, the block would       \
           * be discarded otherwise. This makes small blocks possible for      \
           * faster transmissions of small values. */                          \
          if(status == LRT_RCORE_OK) {                                         \
            handle->block.significant_bytes = handle->byte_counter;            \
            status = sPREFIX##_handle_complete_block(handle);                  \
          }                                                                    \
        }                                                                      \
        handle->byte_counter = 0;                                              \
      } else if(handle->byte_counter >= iBLOCK_SIZE) {                         \
        /* This seems like an overlong message. It should still be able to     \
         * be parsed, if receiving is stopped now. This behaviour can be       \
         * useful in environments like SPI or with harmful inputs. */          \
        if(status == LRT_RCORE_OK) {                                           \
          handle->block.significant_bytes = handle->byte_counter;              \
          status = sPREFIX##_handle_complete_block(handle);                    \
        }                                                                      \
        handle->byte_counter = 0;                                              \
      }                                                                        \
      /* Write the received byte into the block. */                            \
      assert(handle->byte_counter < iBLOCK_SIZE);                              \
      assert(i < length);                                                      \
      handle->block.data[handle->byte_counter++] = data[i];                    \
    }                                                                          \
    /* Also handle finished blocks if there are no additional bytes. */        \
    if(handle->byte_counter > 0 && handle->byte_counter % 8 == 0) {            \
      if(status == LRT_RCORE_OK) {                                             \
        handle->block.significant_bytes = handle->byte_counter;                \
        status = sPREFIX##_handle_complete_block(handle);                      \
      }                                                                        \
      handle->byte_counter = 0;                                                \
    }                                                                          \
    return status;                                                             \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#ifdef __cplusplus
#define LRT_RCOMM_PTR(sPREFIX, sPREFIX_UPPER)                               \
  namespace lrt {                                                           \
  namespace RCore {                                                         \
  struct sPREFIX_UPPER##HandleDeleter                                       \
  {                                                                         \
    void operator()(sPREFIX##_handle_t* handle) { sPREFIX##_free(handle); } \
  };                                                                        \
                                                                            \
  using sPREFIX_UPPER##HandlePtr =                                          \
    std::unique_ptr<sPREFIX##_handle_t, sPREFIX_UPPER##HandleDeleter>;      \
  }                                                                         \
  }
#else
#define LRT_RCOMM_PTR(sPREFIX, sPREFIX_UPPER)
#endif

#endif
