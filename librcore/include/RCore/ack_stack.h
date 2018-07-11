#ifndef LRT_LIBRCORE_ACK_STACK_H
#define LRT_LIBRCORE_ACK_STACK_H

#include "events.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LRT_RCORE_ACK_STACK(sPREFIX, iBLOCK_SIZE, iACK_STACK_SIZE)           \
  typedef struct sPREFIX##_ack_stack_entry_t                                 \
  {                                                                          \
    int8_t sequence_number;                                                  \
    sPREFIX##_block_t block;                                                 \
  } sPREFIX##_ack_stack_entry_t;                                             \
  typedef struct sPREFIX##_ack_stack_t                                       \
  {                                                                          \
    sPREFIX##_ack_stack_entry_t entries[iACK_STACK_SIZE];                    \
  } sPREFIX##_ack_stack_t;                                                   \
  void sPREFIX##_init_ack_stack(sPREFIX##_ack_stack_t* stack)                \
  {                                                                          \
    for(size_t i = 0; i < iACK_STACK_SIZE; ++i) {                            \
      stack->entries[i].sequence_number = -1;                                \
    }                                                                        \
  }                                                                          \
  size_t sPREFIX##_ack_stack_count_pending(sPREFIX##_ack_stack_t* stack)     \
  {                                                                          \
    size_t count = 0;                                                        \
    for(size_t i = 0; i < iACK_STACK_SIZE; ++i) {                            \
      if(stack->entries[i].sequence_number >= 0) {                           \
        ++count;                                                             \
      }                                                                      \
    }                                                                        \
    return count;                                                            \
  }                                                                          \
  lrt_rcore_event_t sPREFIX##_ack_stack_insert(sPREFIX##_ack_stack_t* stack, \
                                               sPREFIX##_block_t* block)     \
  {                                                                          \
    /* Find a free slot in the stack. */                                     \
    sPREFIX##_ack_stack_entry_t* entry = NULL;                               \
    for(size_t i = 0; i < iACK_STACK_SIZE; ++i) {                            \
      if(stack->entries[i].sequence_number < 0) {                            \
        entry = &stack->entries[i];                                          \
        break;                                                               \
      }                                                                      \
    }                                                                        \
    if(entry == NULL) {                                                      \
      return LRT_RCORE_ACK_STACK_FULL;                                       \
    }                                                                        \
    /* Save the block this slot. */                                          \
    entry->sequence_number = sPREFIX##_get_sequence_number(block);           \
    memcpy(&entry->block.data, block->data, iBLOCK_SIZE);                    \
                                                                             \
    return LRT_RCORE_OK;                                                     \
  }                                                                          \
  lrt_rcore_event_t sPREFIX##_ack_stack_remove(sPREFIX##_ack_stack_t* stack, \
                                               sPREFIX##_block_t* block)     \
  {                                                                          \
    for(size_t i = 0; i < iACK_STACK_SIZE; ++i) {                            \
      if(stack->entries[i].sequence_number ==                                \
           sPREFIX##_get_sequence_number(block) &&                           \
         sPREFIX##_get_litecomm_type(&stack->entries[i].block) ==            \
           sPREFIX##_get_litecomm_type(block) &&                             \
         sPREFIX##_get_litecomm_property(&stack->entries[i].block) ==        \
           sPREFIX##_get_litecomm_property(block)) {                         \
        stack->entries[i].sequence_number = -1;                              \
        return LRT_RCORE_OK;                                                 \
      }                                                                      \
    }                                                                        \
    return LRT_RCORE_NO_ACK_ENTRY_FOUND;                                     \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
