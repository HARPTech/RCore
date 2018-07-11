#ifndef LRT_LIBRCORE_RCOMM_H
#define LRT_LIBRCORE_RCOMM_H

#include "ack_stack.h"
#include "callbacks.h"
#include "events.h"
// For some reason, the sequence stack breaks the CPP of the message.h file.
// clang-format off
#include <RCore/librcp/message.h>
#include "sequence_stack.h"
// clang-format on
#include <RCore/librbp/block.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>

#define LRT_RCORE_RCOMM_DEFINE_PROTOCOL(                                       \
  sPREFIX, iBLOCK_SIZE, tMESSAGE, iSTACK_WIDTH, iSTACK_DEPTH, iACK_STACK_SIZE) \
  LRT_LIBRBP_BLOCK_STRUCT(sPREFIX, iBLOCK_SIZE, tMESSAGE)                      \
  LRT_RCORE_CALLBACKS(sPREFIX)                                                 \
  LRT_RCORE_SEQUENCE_STACK(sPREFIX, iBLOCK_SIZE, iSTACK_WIDTH, iSTACK_DEPTH)   \
  LRT_RCORE_ACK_STACK(sPREFIX, iBLOCK_SIZE, iACK_STACK_SIZE)                   \
  LRT_LIBRCP_TYPES(sPREFIX)                                                    \
  typedef struct sPREFIX##_handle_t                                            \
  {                                                                            \
    sPREFIX##_block_t block;                                                   \
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
    sPREFIX##_init_ack_stack(&handle->ack_stack);                              \
    sPREFIX##_init_sequence_stack(&handle->sequence_stack);                    \
    handle->transmit_userdata = NULL;                                          \
    handle->accept_userdata = NULL;                                            \
    handle->byte_counter = 0;                                                  \
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
  lrt_rcore_event_t sPREFIX##_handle_complete_block(                           \
    sPREFIX##_handle_t* handle)                                                \
  {                                                                            \
    assert(handle->accept != NULL);                                            \
    /* Blocks have to be handled according to the configuration of this RComm  \
     * stack. They are handled over the sequence stack. */                     \
    if(sPREFIX##_is_ack(&handle->block)) {                                     \
      /* This block is just an ACKnowledge block. Mark the internal ACK-Stack, \
       * so this block does not get sent again. */                             \
      return sPREFIX##_ack_stack_remove(&handle->ack_stack, &handle->block);   \
    }                                                                          \
    if(sPREFIX##_is_tinyPacket(&handle->block)) {                              \
      /* Can be directly given to the acceptor. */                             \
      handle->accept(&handle->block, handle->accept_userdata);                 \
    } else if(sPREFIX##_is_reliable(&handle->block)) {                         \
      sPREFIX##_sequence_stack_handle_block(&handle->sequence_stack,           \
                                            &handle->block,                    \
                                            handle->accept,                    \
                                            handle->accept_userdata,           \
                                            handle->transmit,                  \
                                            handle->transmit_userdata);        \
    } else {                                                                   \
      /* If the block is not reliable, it can just be given to the acceptor.   \
       */                                                                      \
      handle->accept(&handle->block, handle->accept_userdata);                 \
    }                                                                          \
    return LRT_RCORE_OK;                                                       \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_parse_bytes(                                     \
    sPREFIX##_handle_t* handle, const uint8_t* data, size_t length)            \
  {                                                                            \
    for(size_t i = 0; i < length; ++i) {                                       \
      if(data[i] & LRT_LIBRBP_BLOCK_START_BIT) {                               \
        /* Begin of new block, old block can be given to next handler. */      \
        if(handle->byte_counter > 0 && handle->byte_counter % 8 == 0) {        \
          /* Only if the byte counter is dividable by 8, the block would       \
           * be discarded otherwise. This makes small blocks possible for      \
           * faster transmissions of small values. */                          \
          sPREFIX##_handle_complete_block(handle);                             \
        }                                                                      \
        handle->byte_counter = 0;                                              \
      }                                                                        \
      /* Write the received byte into the block. */                            \
      handle->block.data[handle->byte_counter++] = data[i];                    \
    }                                                                          \
    /* Also handle finished blocks if there are no additional bytes. */        \
    if(handle->byte_counter > 0 && handle->byte_counter % 8 == 0) {            \
      sPREFIX##_handle_complete_block(handle);                                 \
    }                                                                          \
    handle->byte_counter = 0;                                                  \
    return LRT_RCORE_OK;                                                       \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
