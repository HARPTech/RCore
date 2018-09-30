#include "../include/RCore/ack_stack.h"
#include "../../librsp/include/RCore/librsp/litecomm.h"
#include "../include/RCore/internal/hashtable.h"
#include "../include/RCore/rcomm.h"

#include <assert.h>
#include <klib/khash.h>
#include <klib/klist.h>

KLIST_INIT(message_list, lrt_rbp_message_t, LRT_LIBRBP_MESSAGE_LIST_FREER)

typedef struct rcomm_ack_stack_entry_t
{
  size_t queue_size;
  uint32_t ns_since_last_ack;
  klist_t(message_list) * messages;
} rcomm_ack_stack_entry_t;

LRT_LIBRCORE_HASHTABLE_INIT(entry_map, rcomm_ack_stack_entry_t)

static void
free_stack_entry(rcomm_ack_stack_entry_t* entry)
{
  if(entry->messages != NULL) {
    // Free all contained messages.
    kl_destroy(message_list, entry->messages);
  }
}

typedef struct lrt_rcore_ack_stack_t
{
  size_t entries_in_use;
  size_t messages_in_use;
  size_t maximum_entries_in_use;
  size_t maximum_queue_size;

  size_t ack_ns_avg_sampling_rate;
  uint32_t ack_ns_avg;

  uint32_t ack_resending_threshold;

  khash_t(entry_map) * entries;
} lrt_rcore_ack_stack_t;

static void
recalculate_ack_resending_threshold(lrt_rcore_ack_stack_t* stack,
                                    uint32_t ns_since_last_ack)
{
  stack->ack_ns_avg -= stack->ack_ns_avg / stack->ack_ns_avg_sampling_rate;
  stack->ack_ns_avg += ns_since_last_ack / stack->ack_ns_avg_sampling_rate;

  stack->ack_resending_threshold = stack->ack_ns_avg * 3 / 2;
}

static void
init_stack_entry(rcomm_ack_stack_entry_t* entry)
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
  entry->ns_since_last_ack = 0;
}

static rcomm_ack_stack_entry_t*
get_or_create_entry_from_stack(lrt_rcore_ack_stack_t* stack,
                               uint8_t lType,
                               uint16_t lProp)
{
  assert(stack != NULL);

  int16_t key = lrt__hashtable_key_from_property(lType, lProp);

  // Try to find the entry first.
  khiter_t it = kh_get(entry_map, stack->entries, key);

  if(it != kh_end(stack->entries)) {
    return &kh_val(stack->entries, it);
  } else {
    // Could not find the entry in the hash map, create a new entry in the map
    // if there is space left.
    if(stack->entries_in_use < stack->maximum_entries_in_use) {
      int ret = 0;
      it = kh_put(entry_map, stack->entries, key, &ret);
      switch(ret) {
        case -1:
          return NULL;
        case 0:
          // Already present.
          break;
        case 1: {
          // Never used. Initialise it with zeroes.
          rcomm_ack_stack_entry_t* e = &kh_val(stack->entries, it);
          e->messages = NULL;
          e->queue_size = 0;
          // Fall through to case 2, which is a reuse. No break needed.
        }
        case 2:
          // Reuse previously initialised entry.
          init_stack_entry(&kh_val(stack->entries, it));
          break;
      }

      ++stack->entries_in_use;

      return &kh_val(stack->entries, it);
    } else {
      return NULL;
    }
  }
}

static lrt_rcore_event_t
pop_message(lrt_rcore_ack_stack_t* stack,
            uint8_t lType,
            uint16_t lProp,
            uint16_t sequence_number)
{
  // Find the respective entry.
  khiter_t it = kh_get(
    entry_map, stack->entries, lrt__hashtable_key_from_property(lType, lProp));

  if(it == kh_end(stack->entries)) {
    // No entry found, this request is invalid.
    return LRT_RCORE_NO_ACK_ENTRY_FOUND;
  }

  rcomm_ack_stack_entry_t* entry = &kh_val(stack->entries, it);

  // Set the new head to the next message.
  if(entry->messages->head != entry->messages->tail) {
    kliter_t(message_list)* p = entry->messages->head;

    // The sequence numbers always have to be in the right order logically.
    if(rcomm_message_get_sequence_number(&p->data) != sequence_number) {
      return LRT_RCORE_NO_ACK_ENTRY_FOUND;
    }

    p->data.length = 0;
    kmp_free(message_list, entry->messages->mp, p);

    entry->messages->head = entry->messages->head->next;
  }

  // Reduce sizes.
  --entry->queue_size;
  --stack->messages_in_use;

  recalculate_ack_resending_threshold(stack, entry->ns_since_last_ack);
  entry->ns_since_last_ack = 0;

  if(entry->queue_size == 0) {
    kh_del(entry_map, stack->entries, it);
    --stack->entries_in_use;
  }
  return LRT_RCORE_OK;
}

static lrt_rcore_event_t
push_message(lrt_rcore_ack_stack_t* stack,
             uint8_t lType,
             uint16_t lProp,
             const lrt_rbp_message_t* message)
{
  rcomm_ack_stack_entry_t* entry =
    get_or_create_entry_from_stack(stack, lType, lProp);

  if(entry == NULL) {
    return LRT_RCORE_ACK_STACK_MAP_FULL;
  }

  if(entry->queue_size < stack->maximum_queue_size) {
    lrt_rbp_message_copy(kl_pushp(message_list, entry->messages), message);
    ++entry->queue_size;
    return LRT_RCORE_OK;
  } else {
    return LRT_RCORE_ACK_STACK_QUEUE_FULL;
  }
}

lrt_rcore_event_t
lrt_rcore_ack_stack_insert(lrt_rcore_ack_stack_t* stack,
                           const lrt_rbp_message_t* message)
{
  return push_message(stack,
                      rcomm_message_get_litecomm_type(message),
                      rcomm_message_get_litecomm_property(message),
                      message);
}

lrt_rcore_event_t
lrt_rcore_ack_stack_remove(lrt_rcore_ack_stack_t* stack,
                           const lrt_rbp_message_t* message)
{
  return pop_message(stack,
                     rcomm_message_get_litecomm_type(message),
                     rcomm_message_get_litecomm_property(message),
                     rcomm_message_get_sequence_number(message));
}

lrt_rcore_event_t
lrt_rcore_ack_stack_tick(lrt_rcore_ack_stack_t* stack,
                         uint32_t ns_since_last_tick,
                         rcomm_handle_t* handle)
{
  khiter_t k;
  rcomm_ack_stack_entry_t* entry = NULL;
  lrt_rcore_event_t status = LRT_RCORE_OK;

  for(k = kh_begin(stack->entries);
      k != kh_end(stack->entries) && status == LRT_RCORE_OK;
      ++k) {
    if(kh_exist(stack->entries, k)) {
      // Entry exists, it can be analysed for potential re-sending.
      entry = &kh_val(stack->entries, k);

      entry->ns_since_last_ack += ns_since_last_tick;

      if(entry->ns_since_last_ack > stack->ack_resending_threshold) {
        // Resend the oldest message from the queue of this entry.
        if(entry->messages->head != NULL) {
          status = rcomm_transmit_message(handle, &entry->messages->head->data);
        }
      }
    }
  }
  return status;
}
lrt_rcore_ack_stack_t*
lrt_rcore_ack_stack_init(size_t stack_size,
                         size_t maximum_stack_size,
                         size_t maximum_queue_size,
                         size_t ack_ns_avg_sampling_rate)
{
  assert(stack_size < maximum_stack_size);

  lrt_rcore_ack_stack_t* stack = calloc(sizeof(lrt_rcore_ack_stack_t), 1);
  stack->maximum_queue_size = maximum_queue_size;
  stack->maximum_entries_in_use = maximum_stack_size;

  stack->ack_ns_avg = 10;
  stack->ack_ns_avg_sampling_rate = ack_ns_avg_sampling_rate;

  stack->entries = kh_init(entry_map);

  return stack;
}

void
lrt_rcore_ack_stack_free(lrt_rcore_ack_stack_t* stack)
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

  free(stack);
}
