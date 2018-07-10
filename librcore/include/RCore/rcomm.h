#ifndef LRT_LIBRCORE_RCOMM_H
#define LRT_LIBRCORE_RCOMM_H

#include "events.h"
#include "sequence_stack.h"
#include <RCore/librbp/block.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>

#define LRT_RCORE_RCOMM_DEFINE_PROTOCOL(                                      \
  sPREFIX, iBLOCK_SIZE, tMESSAGE, iSTACK_WIDTH, iSTACK_DEPTH)                 \
  LRT_LIBRBP_BLOCK_STRUCT(sPREFIX, iBLOCK_SIZE, tMESSAGE)                     \
  LRT_RCORE_SEQUENCE_STACK(sPREFIX, iSTACK_WIDTH, iSTACK_DEPTH)               \
  typedef struct sPREFIX##_handle_t                                           \
  {                                                                           \
    sPREFIX##_block_t block;                                                  \
    size_t byte_counter;                                                      \
    sPREFIX##_sequence_stack_t sequence_stack;                                \
    /* Callbacks */                                                           \
    void* transmit_userdata;                                                  \
    void (*transmit)(const uint8_t*, size_t, void*);                          \
    void* accept_userdata;                                                    \
    void (*accept)(sPREFIX##_block_t*, void*);                                \
  } sPREFIX##_handle_t;                                                       \
  lrt_rcore_event_t sPREFIX##_handle_complete_block(                          \
    sPREFIX##_handle_t* handle)                                               \
  {                                                                           \
    /* Blocks have to be handled according to the configuration of this RComm \
     * stack. They are handled over the sequence stack. */                    \
    if(sPREFIX##_is_tinyPacket(&handle->block)) {                             \
      /* Can be directly given to the acceptor. */                            \
      assert(handle->accept != NULL);                                         \
      (handle->accept)(&handle->block, handle->accept_userdata);              \
    } else {                                                                  \
    }                                                                         \
  }                                                                           \
  lrt_rcore_event_t sPREFIX##_parse_bytes(                                    \
    sPREFIX##_handle_t* handle, const uint8_t* data, size_t length)           \
  {                                                                           \
    for(size_t i = 0; i < length; ++i) {                                      \
      if(data[i] & LRT_LIBRBP_BLOCK_START_BIT) {                              \
        /* Begin of new block, old block can be given to next handler. */     \
        if(handle->byte_counter % 8 == 0) {                                   \
          /* Only if the byte counter is dividable by 8, the block would      \
           * be discarded otherwise. This makes small blocks possible for     \
           * faster transmissions of small values. */                         \
          sPREFIX##_handle_complete_block(handle);                            \
        }                                                                     \
        handle->byte_counter = 0;                                             \
        /* Write the received byte into the block. */                         \
        handle->block.data[handle->byte_counter++] = data[i];                 \
      }                                                                       \
    }                                                                         \
    /* Also handle finished blocks if there are no additional bytes. */       \
    if(handle->byte_counter % 8 == 0) {                                       \
      sPREFIX##_handle_complete_block(handle);                                \
    }                                                                         \
    handle->byte_counter = 0;                                                 \
    return LRT_RCORE_OK;                                                      \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
