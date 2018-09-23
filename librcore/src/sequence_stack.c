#include "../include/RCore/sequence_stack.h"
#include "../../librsp/include/RCore/librsp/flags.h"
#include "../../librsp/include/RCore/librsp/litecomm.h"
#include "../include/RCore/internal/hashtable.h"
#include "../include/RCore/rcomm.h"
#include <assert.h>
#include <klib/khash.h>
#include <klib/klist.h>

KLIST_INIT(message_list, lrt_rbp_message_t, LRT_LIBRBP_MESSAGE_LIST_FREER)

typedef struct lrt_rcore_sequence_stack_entry_t
{
  int16_t expected_sequence_number;
  size_t queue_size;
  klist_t(message_list) * messages;
} lrt_rcore_sequence_stack_entry_t;

static void
free_stack_entry(lrt_rcore_sequence_stack_entry_t* entry)
{
  if(entry->messages != NULL) {
    // Free all contained messages.
    kl_destroy(message_list, entry->messages);
  }
}

LRT_LIBRCORE_HASHTABLE_INIT(entry_map, lrt_rcore_sequence_stack_entry_t)

typedef struct lrt_rcore_sequence_stack_t
{
  size_t entries_in_use;
  size_t messages_in_use;
  size_t maximum_entries_in_use;
  size_t maximum_queue_size;

  khash_t(entry_map) * entries;
} lrt_rcore_sequence_stack_t;

lrt_rcore_sequence_stack_t*
lrt_rcore_sequence_stack_init()
{
  lrt_rcore_sequence_stack_t* stack =
    calloc(sizeof(lrt_rcore_sequence_stack_t), 1);
  return stack;
}

void
lrt_rcore_sequence_stack_free(lrt_rcore_sequence_stack_t* stack)
{
  khiter_t k;
  if(stack->entries != NULL) {
    for(k = kh_begin(stack->entries); k != kh_end(stack->entries); ++k) {
      if(kh_exist(stack->entries, k)) {
        free_stack_entry(&kh_val(stack->entries, k));
      }
    }
  }
  kh_destroy(entry_map, stack->entries);
}

static void
init_stack_entry(lrt_rcore_sequence_stack_entry_t* entry)
{
  if(entry->messages == NULL) {
    // Initiate the messages list.
    entry->messages = kl_init(message_list);
  } else {
    // Empty the messages list completely.
    lrt_rbp_message_t* msg = NULL;
    while(kl_shift(message_list, entry->messages, msg) == 0) {
      assert(msg != NULL);
    }
  }
  entry->queue_size = 0;
  entry->expected_sequence_number = -1;
}

static int16_t
get_next_expected_sequence_number(int16_t sequence_number)
{
  ++sequence_number;

  // This check is needed because it prohibits signed vs unsigned integer
  // overflow errors.
  if(sequence_number >= 0x0FFF) {
    sequence_number = 0;
  }

  return sequence_number;
}

static lrt_rcore_event_t
search_stack_and_insert(lrt_rcore_sequence_stack_t* stack,
                        lrt_rbp_message_t* message,
                        rcomm_accept_block_cb acceptor,
                        void* acceptor_userdata,
                        rcomm_handle_t* handle)
{
  /* Try to find the entry in the stack's hash-map or create a new one. Streams
   * have to have a start and an end.*/
  LRT_LIBRCORE_HASHTABLE_KEY_TYPE key = lrt__hashtable_key_from_property(
    rcomm_message_get_litecomm_type(message),
    rcomm_message_get_litecomm_property(message));

  khint_t it = kh_get(entry_map, stack->entries, key);

  if(it == kh_end(stack->entries)) {
    // No entry found! A new one for this property has to be inserted if there
    // is space left.
    int ret = 0;
    it = kh_put(entry_map, stack->entries, key, &ret);
    if(ret == -1) {
      return LRT_RCORE_STACK_FULL;
    }
  }

  // The entry has been found, stream processing can start.
  assert(it != kh_end(stack->entries));
  lrt_rcore_sequence_stack_entry_t* entry = &kh_val(stack->entries, it);
  assert(entry != NULL);

  int16_t sequence_number = (int16_t)rcomm_message_get_sequence_number(message);

  // Check if this is the start of a stream and set the sequence number
  // accordingly.
  if(rcomm_message_has_flag(message, LRT_LIBRSP_STREAM_START)) {
    entry->expected_sequence_number = sequence_number;
  }

  if(sequence_number == entry->expected_sequence_number) {
    // The received message matches with the expected sequence number. It can be
    // used directly and given to the acceptor.
    entry->expected_sequence_number =
      get_next_expected_sequence_number(sequence_number);

    return acceptor(message, acceptor_userdata);
  }

  // Sequence number mismatch! Maybe a message was dropped or they were
  // reordered during transfer. To still be able to receive the message, no ACK
  // will be sent and it has to be waited until the other side sends the message
  // again.

  // Test if there is even still space left in the queue.
  if(entry->queue_size >= stack->maximum_queue_size) {
    return LRT_RCORE_STACK_DEPTH_EXHAUSTED;
  }

  ++entry->queue_size;

  // Search for (logical) last message in the internal list of messages.
  kliter_t(message_list) * p;
  kliter_t(message_list)* last_p = NULL;
  bool inserted = false;

  for(p = entry->messages->head; p != entry->messages->tail;
      last_p = p, p = p->next) {
    if(rcomm_message_get_sequence_number(&p->data) > sequence_number) {
      kliter_t(message_list)* new_p = NULL;

      new_p = kmp_alloc(message_list, entry->messages->mp);
      lrt_rbp_message_copy(&new_p->data, message);
      new_p->next = p;

      if(last_p == NULL) {
        entry->messages->head = new_p;
      } else {
        last_p->next = new_p;
      }

      inserted = true;
      break;
    }
  }

  if(!inserted) {
    // Not inserted yet, this message has to go at the end of the list.
    lrt_rbp_message_copy(kl_pushp(message_list, entry->messages), message);
  }

  // Check the newly arranged list if it is correct enough to extract fully
  // formed messages from it.
  lrt_rcore_event_t status = LRT_RCORE_OK;

  for(p = entry->messages->head;
      p != entry->messages->tail &&
      entry->expected_sequence_number ==
        rcomm_message_get_sequence_number(&p->data) &&
      status == LRT_RCORE_OK;
      p = p->next) {
    // The expected sequence number and the current sequence number of p match,
    // so the list can be shifted and the lowest element can be given to the
    // acceptor. This also updates the new expected sequence number in the
    // entry.
    lrt_rbp_message_t* msg = NULL;
    if(kl_shift(message_list, entry->messages, msg) != 0) {
      return LRT_RCORE_INTERNAL_SEQUENCE_STACK_ERROR;
    }
    assert(msg != NULL);

    // Queue size is reduced by 1.
    --entry->queue_size;

    entry->expected_sequence_number =
      get_next_expected_sequence_number(sequence_number);

    status = acceptor(msg, acceptor_userdata);

    if(rcomm_message_has_flag(msg, LRT_LIBRSP_STREAM_END)) {
      // The ending message has just been popped! This means, this stream is
      // finished and the entry can be purged.

      init_stack_entry(entry);
      kh_del(entry_map, stack->entries, it);

      break;
    }
  }

  return status;
}

lrt_rcore_event_t
lrt_rcore_sequence_stack_handle_message(lrt_rcore_sequence_stack_t* stack,
                                        lrt_rbp_message_t* message,
                                        rcomm_accept_block_cb acceptor,
                                        void* acceptor_userdata,
                                        rcomm_handle_t* handle)
{
  uint8_t stream_bits =
    rcomm_get_packetTypeBits(message) & LRT_LIBRSP_STREAM_TINY;
  switch(stream_bits) {
    case 0:
    case LRT_LIBRSP_STREAM_END:
    case LRT_LIBRSP_STREAM_START:
      return search_stack_and_insert(
        stack, message, acceptor, acceptor_userdata, handle);
      break;
    default:
      // Other message types are not handled by the sequence stack. They are
      // instead handled directly by rcomm.
      break;
  }
  return LRT_RCORE_OK;
}
