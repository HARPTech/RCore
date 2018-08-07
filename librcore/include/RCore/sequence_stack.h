#ifndef LRT_LIBRCORE_SEQUENCE_STACK_H
#define LRT_LIBRCORE_SEQUENCE_STACK_H

#include "callbacks.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>

#define LRT_RCORE_SEQUENCE_STACK(                                              \
  sPREFIX, iBLOCK_SIZE, iSTACK_WIDTH, iSTACK_DEPTH)                            \
  typedef struct sPREFIX##_sequence_stack_entry_t                              \
  {                                                                            \
    /* The LiteCommType of this entry. Negative Numbers mean, the entry is     \
     * unused. */                                                              \
    int8_t liteCommType;                                                       \
    int16_t liteCommProperty;                                                  \
    /* Next expected sequence number, negative numbers mean there is no        \
     * expected sequence number and the stack has to be traversed. */          \
    int8_t expectedSequenceNumber;                                             \
    sPREFIX##_block_t blocks[iSTACK_DEPTH];                                    \
    size_t stack_counter;                                                      \
  } sPREFIX##_sequence_stack_entry_t;                                          \
  typedef struct sPREFIX##_sequence_stack_t                                    \
  {                                                                            \
    sPREFIX##_sequence_stack_entry_t entries[iSTACK_WIDTH];                    \
  } sPREFIX##_sequence_stack_t;                                                \
  void sPREFIX##_init_sequence_stack(sPREFIX##_sequence_stack_t* stack)        \
  {                                                                            \
    for(size_t i = 0; i < iSTACK_WIDTH; ++i) {                                 \
      stack->entries[i].expectedSequenceNumber = -1;                           \
      stack->entries[i].liteCommType = -1;                                     \
      stack->entries[i].liteCommProperty = -1;                                 \
      stack->entries[i].stack_counter = 0;                                     \
    }                                                                          \
  }                                                                            \
  int sPREFIX##_sequence_stack_cmp(const void* left_void,                      \
                                   const void* right_void)                     \
  {                                                                            \
    sPREFIX##_block_t* left = (sPREFIX##_block_t*)left_void;                   \
    sPREFIX##_block_t* right = (sPREFIX##_block_t*)right_void;                 \
    uint8_t left_seq_num = sPREFIX##_get_sequence_number(left);                \
    uint8_t right_seq_num = sPREFIX##_get_sequence_number(right);              \
    /* This calculates the distance of the two blocks and tries to put them    \
     * into the correct order. */                                              \
    int8_t dist = left_seq_num - right_seq_num;                                \
    return dist;                                                               \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_sequence_stack_search_and_insert(                \
    sPREFIX##_sequence_stack_t* stack,                                         \
    sPREFIX##_block_t* block,                                                  \
    sPREFIX##_accept_block_cb acceptor,                                        \
    void* acceptor_userdata,                                                   \
    sPREFIX##_transmit_data_cb transmit_data,                                  \
    void* transmit_userdata)                                                   \
  {                                                                            \
    /* Iterate through all slots to find the entry with matching LiteCommType  \
     * and LiteCommProperty to find the next expected sequence number, match   \
     * it with the sequence number of the received block and, if the do not    \
     * match, insert the current block into the stack. If there already are    \
     * blocks in the stack entry, reorder them according to their sequence     \
     * number pairs. */                                                        \
    sPREFIX##_sequence_stack_entry_t* entry = NULL;                            \
    for(size_t i = 0; i < iSTACK_WIDTH; ++i) {                                 \
      if(stack->entries[i].liteCommType >= 0 &&                                \
         stack->entries[i].liteCommType ==                                     \
           sPREFIX##_get_litecomm_type(block) &&                               \
         stack->entries[i].liteCommProperty ==                                 \
           sPREFIX##_get_litecomm_property(block)) {                           \
        /* This entry matches the current block, proceed with this one. */     \
        entry = &stack->entries[i];                                            \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if(entry == NULL) {                                                        \
      /* An empty entry has to be found for the current block. */              \
      for(size_t i = 0; i < iSTACK_WIDTH; ++i) {                               \
        if(stack->entries[i].liteCommType < 0) {                               \
          /* Empty entry found, using this one. */                             \
          entry = &stack->entries[i];                                          \
          entry->liteCommType = sPREFIX##_get_litecomm_type(block);            \
          entry->liteCommProperty = sPREFIX##_get_litecomm_property(block);    \
          if(sPREFIX##_is_sStart(block)) {                                     \
            /* If the packet is the start of a stream, the next expected       \
             * sequence number is the current sequence number. */              \
            entry->expectedSequenceNumber =                                    \
              sPREFIX##_get_sequence_number(block);                            \
            /* No stack operation had to be done, so the stack counter can be  \
             * initialised to 0. */                                            \
            entry->stack_counter = 0;                                          \
          } else {                                                             \
            /* If the packet is not the start of the stream, a block was       \
             * already lost. The packet is put into the stack at position 0    \
             * (later on) and the stack counter is set to 1.                   \
             */                                                                \
            entry->stack_counter = 0;                                          \
            entry->expectedSequenceNumber = -1;                                \
          }                                                                    \
          break;                                                               \
        }                                                                      \
      }                                                                        \
      if(entry == NULL) {                                                      \
        /* No free entry found! Return error code. */                          \
        return LRT_RCORE_STACK_FULL;                                           \
      }                                                                        \
    }                                                                          \
    /* Check, if the expected sequence number matches the new block. If it     \
     * does, no copy operation has to be done, because the block can be given  \
     * directly to the acceptor. */                                            \
    if(entry->expectedSequenceNumber ==                                        \
       sPREFIX##_get_sequence_number(block)) {                                 \
      entry->expectedSequenceNumber =                                          \
        sPREFIX##_get_sequence_number(block) + 1;                              \
                                                                               \
      if(sPREFIX##_is_sEnd(block)) {                                           \
        /* If this packet is the end of the stream, the slot can directly be   \
         * made free again. */                                                 \
        entry->liteCommType = -1;                                              \
      }                                                                        \
                                                                               \
      acceptor(block, acceptor_userdata);                                      \
                                                                               \
      if(sPREFIX##_is_reliable(block)) {                                       \
        /* ACKnowledge the packet by changing the ACK bit and only sending 8   \
         * bytes. */                                                           \
        sPREFIX##_set_ack(block, true);                                        \
        transmit_data(block->data, transmit_userdata, 8u);                     \
      }                                                                        \
    } else {                                                                   \
      /* The packet has to be inserted into the sequence stack according to    \
       * the sequence numbers. Order of events:                                \
       * 1. Insert to top                                                      \
       * 2. Sort,                                                              \
       * 3. Check                                                              \
       * 4. (If check passes), accept blocks up to the point it does not pass  \
       * anymore                                                               \
       * 5. Move remaining blocks to the front. */                             \
      if(entry->stack_counter == iSTACK_DEPTH) {                               \
        return LRT_RCORE_STACK_DEPTH_EXHAUSTED;                                \
      }                                                                        \
      assert(block->significant_bytes <= iBLOCK_SIZE &&                        \
             block->significant_bytes > 0);                                    \
      memcpy(entry->blocks[entry->stack_counter].data,                         \
             block->data,                                                      \
             block->significant_bytes);                                        \
      entry->blocks[entry->stack_counter].significant_bytes =                  \
        block->significant_bytes;                                              \
      ++entry->stack_counter;                                                  \
      qsort(entry->blocks,                                                     \
            entry->stack_counter,                                              \
            sizeof(sPREFIX##_block_t),                                         \
            sPREFIX##_sequence_stack_cmp);                                     \
      /* Remove duplicates and find stream start. 100 is not reachable by      \
       * normal means, which makes it the default value without similar        \
       * sequence numbers in received blocks. */                               \
      for(size_t i = 0, last_sequence_number = 100u; i < entry->stack_counter; \
          ++i) {                                                               \
        if(sPREFIX##_is_sStart(&entry->blocks[i])) {                           \
          entry->expectedSequenceNumber =                                      \
            sPREFIX##_get_sequence_number(&entry->blocks[i]);                  \
        }                                                                      \
        if(sPREFIX##_get_sequence_number(&entry->blocks[i]) ==                 \
           last_sequence_number) {                                             \
          memmove(entry->blocks + i - 1,                                       \
                  entry->blocks + i,                                           \
                  (entry->stack_counter - i) * sizeof(sPREFIX##_block_t));     \
          --entry->stack_counter;                                              \
        }                                                                      \
        last_sequence_number =                                                 \
          sPREFIX##_get_sequence_number(&entry->blocks[i]);                    \
      }                                                                        \
    }                                                                          \
    sPREFIX##_block_t* tmp_block = NULL;                                       \
    while(entry->stack_counter > 0 &&                                          \
          entry->expectedSequenceNumber ==                                     \
            sPREFIX##_get_sequence_number(&entry->blocks[0])) {                \
      tmp_block = &entry->blocks[0];                                           \
      entry->expectedSequenceNumber =                                          \
        sPREFIX##_get_sequence_number(tmp_block) + 1;                          \
      acceptor(tmp_block, acceptor_userdata);                                  \
                                                                               \
      if(sPREFIX##_is_reliable(tmp_block)) {                                   \
        /* ACKnowledge the packet by changing the ACK bit and only sending 8   \
         * bytes. */                                                           \
        sPREFIX##_set_ack(tmp_block, true);                                    \
        transmit_data(tmp_block->data, transmit_userdata, 8u);                 \
      }                                                                        \
      if(sPREFIX##_is_sEnd(tmp_block)) {                                       \
        /* If this packet is the end of the stream, the slot can directly be   \
         * made free again. */                                                 \
        entry->liteCommType = -1;                                              \
        return LRT_RCORE_OK;                                                   \
      }                                                                        \
      /* The stack counter can be counted down by one, because one element     \
       * is going to be removed. */                                            \
      --entry->stack_counter;                                                  \
      /* Move remaining blocks to the front, beginning at the 1st element up   \
       * to the new value of stack_counter. */                                 \
      memmove(entry->blocks,                                                   \
              entry->blocks + 1,                                               \
              entry->stack_counter * sizeof(sPREFIX##_block_t));               \
    }                                                                          \
    return LRT_RCORE_OK;                                                       \
  }                                                                            \
  lrt_rcore_event_t sPREFIX##_sequence_stack_handle_block(                     \
    sPREFIX##_sequence_stack_t* stack,                                         \
    sPREFIX##_block_t* block,                                                  \
    sPREFIX##_accept_block_cb acceptor,                                        \
    void* acceptor_userdata,                                                   \
    sPREFIX##_transmit_data_cb transmit_data,                                  \
    void* transmit_userdata)                                                   \
  {                                                                            \
    /* Switch on the Stream Start / Stream End Bits */                         \
    switch(sPREFIX##_get_packetStreamBits(block)) {                            \
      case LRT_LIBRSP_STREAM_TINY:                                             \
        /* Tiny packets are the same as stream start packets, just without     \
         * following data. They can be given directly to the acceptor without  \
         * involving the more expensive stack for queue management.            \
         */                                                                    \
        acceptor(block, acceptor_userdata);                                    \
        if(sPREFIX##_is_reliable(block)) {                                     \
          /* ACKnowledge the packet by changing the ACK bit and only sending 8 \
           * bytes. */                                                         \
          sPREFIX##_set_ack(block, true);                                      \
          return transmit_data(block->data, transmit_userdata, 8u);            \
        }                                                                      \
        break;                                                                 \
        /* Same handling for streaming packets - cases are differentiated      \
         * inside the stack logic. */                                          \
      case LRT_LIBRSP_STREAM_RUNNING:                                          \
      case LRT_LIBRSP_STREAM_END:                                              \
      case LRT_LIBRSP_STREAM_START: {                                          \
        /* When the stream starts, the packet is already correct and can be    \
         * given to the block acceptor callback, but only if there is space at \
         * the stack. The stack handles sending ACKs and accepting blocks. */  \
        lrt_rcore_event_t status =                                             \
          sPREFIX##_sequence_stack_search_and_insert(stack,                    \
                                                     block,                    \
                                                     acceptor,                 \
                                                     acceptor_userdata,        \
                                                     transmit_data,            \
                                                     transmit_userdata);       \
        if(status != LRT_RCORE_OK) {                                           \
          return status;                                                       \
        }                                                                      \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    return LRT_RCORE_OK;                                                       \
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
