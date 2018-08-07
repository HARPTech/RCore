#define LRT_RCORE_DEBUG
#include "../include/RCore/rcomm.h"
#include <RCore/librcp/message.h>
#include <RCore/librsp/stream_message.h>
#include <criterion/criterion.h>
#include <criterion/logging.h>

#define iBLOCKSIZE 8u

LRT_RCORE_RCOMM_DEFINE_PROTOCOL(comm,
                                iBLOCKSIZE,
                                LRT_LIBRSP_STREAM_MESSAGE,
                                3u,
                                8u,
                                64u)

comm_handle_t source;
comm_handle_t target;

typedef struct transmit_userdata_t
{
  const char* src_name;
  const char* tgt_name;
  comm_handle_t* src;
  comm_handle_t* tgt;
  int ack_pending_check;
} transmit_userdata_t;

typedef struct acceptor_userdata_t
{
  int8_t int8_data;
  int8_t int8_data_check;
  int16_t int16_data;
  int16_t int16_data_check;
  int32_t int32_data;
  int32_t int32_data_check;
  int64_t int64_data;
  int64_t int64_data_check;
  uint64_t uint64_data;
  uint64_t uint64_data_check;

  uint8_t liteCommType;
  uint16_t liteCommProperty;

  int16_t counter;

  lrt_rcp_message_type_t messageType;

  int whatToCheck;
} acceptor_userdata_t;

lrt_rcore_event_t
transmit_data_cb(const uint8_t* data, void* userdata, size_t bytes)
{
  transmit_userdata_t* transmit = (transmit_userdata_t*)userdata;

  // Ugly hack for debugging purposes: Print out the blocks transmitted.
  comm_block_t block;
  comm_init_block(&block);
  memcpy(block.data, data, bytes);
  cr_log_warn("%u Bytes Transmitted from %s to %s: %s",
              (unsigned int)bytes,
              transmit->src_name,
              transmit->tgt_name,
              comm_to_str(&block));

  if(transmit->ack_pending_check >= 0) {
    cr_assert_eq(comm_ack_stack_count_pending(&transmit->src->ack_stack),
                 transmit->ack_pending_check);
  }
  lrt_rcore_event_t status = LRT_RCORE_OK;
  status = comm_parse_bytes(transmit->tgt, data, bytes);
  if(status != LRT_RCORE_OK) {
    cr_log_error("Status after transmitting not LRT_RCORE_OK! Status: %d - "
                 "Lookup this code in RCore/events.h!",
                 (int)status);
  }
  cr_assert_eq(status, LRT_RCORE_OK);
  return status;
}
lrt_rcore_event_t
accept_block_cb(comm_block_t* block, void* userdata)
{
  acceptor_userdata_t* accept = (acceptor_userdata_t*)userdata;

  if(comm_is_sStart(block)) {
    // The counter must be -1 at the start.
    accept->counter = -1;

    cr_assert_eq(comm_get_litecomm_message_type(block), accept->messageType);
  }

  cr_assert_eq(comm_get_litecomm_type(block), accept->liteCommType);
  cr_assert_eq(comm_get_litecomm_property(block), accept->liteCommProperty);

  // Write the block into the data fields.
  switch(accept->whatToCheck) {
    case 0:// Check int8
      accept->counter =
        comm_get_data_Int8(block, &accept->int8_data, accept->counter);
      break;
    case 1:// Check int16
      accept->counter =
        comm_get_data_Int16(block, &accept->int16_data, accept->counter);
      break;
    case 2:// Check int32
      accept->counter =
        comm_get_data_Int32(block, &accept->int32_data, accept->counter);
      break;
    case 3:// Check int64
      accept->counter =
        comm_get_data_Int64(block, &accept->int64_data, accept->counter);
      break;
    case 4:// Check uint64
      accept->counter =
        comm_get_data_Uint64(block, &accept->uint64_data, accept->counter);
      break;
  }

  if(comm_is_sEnd(block)) {
    // This block is the end of the stream!
    cr_log_info("Accepting block of finished stream! Data: %s",
                comm_to_str(block));
    switch(accept->whatToCheck) {
      case 0:// Check int8
        cr_assert_eq(accept->int8_data, accept->int8_data_check);
        break;
      case 1:// Check int16
        cr_assert_eq(accept->int16_data, accept->int16_data_check);
        break;
      case 2:// Check int32
        cr_assert_eq(accept->int32_data, accept->int32_data_check);
        break;
      case 3:// Check int64
        cr_assert_eq(accept->int64_data, accept->int64_data_check);
        break;
      case 4:// Check uint64
        cr_assert_eq(accept->uint64_data, accept->uint64_data_check);
        break;
    }
  } else {
    cr_log_info("Accepting intermediate block! Data: %s", comm_to_str(block));
  }

  return LRT_RCORE_OK;
}

transmit_userdata_t sourceToTarget = { .src_name = "Source",
                                       .tgt_name = "Target",
                                       .src = &source,
                                       .tgt = &target,
                                       .ack_pending_check = -1 };
transmit_userdata_t targetToSource = { .src_name = "Target",
                                       .tgt_name = "Source",
                                       .src = &target,
                                       .tgt = &source,
                                       .ack_pending_check = -1 };

acceptor_userdata_t acceptor_userdata = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void
setup(void)
{
  comm_init(&source);
  comm_init(&target);

  comm_set_accept_cb(&source, &accept_block_cb, &acceptor_userdata);
  comm_set_accept_cb(&target, &accept_block_cb, &acceptor_userdata);

  comm_set_transmit_cb(&source, &transmit_data_cb, &sourceToTarget);
  comm_set_transmit_cb(&target, &transmit_data_cb, &targetToSource);
}
static void
teardown(void)
{}

Test(rcore, tiny_unreliable, .init = setup, .fini = teardown)
{
  comm_block_t block;
  comm_init_block(&block);

  comm_set_ack(&block, false);
  comm_set_sStart(&block, true);
  comm_set_sEnd(&block, true);
  comm_set_reliable(&block, false);

  cr_assert(comm_is_tinyPacket(&block));

  // Set the contents: LiteCommType 0, LiteCommProperty: 10, Int8: 123
  comm_set_litecomm_type(&block, 0);
  comm_set_litecomm_property(&block, 10);
  acceptor_userdata.liteCommType = 0;
  acceptor_userdata.liteCommProperty = 10;
  acceptor_userdata.int8_data_check = 123;
  acceptor_userdata.whatToCheck = 0;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_UPDATE;
  int16_t counter = 0;

  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UPDATE);

  counter = comm_set_data_Uint8(&block, 123, -1);
  cr_assert_eq(counter, 0);

  cr_log_info("Transmit Test Block. Data: %s", comm_to_str(&block));
  comm_send_block(&source, &block, 0);

  // Set the contents: LiteCommType 0, LiteCommProperty: 10, Int8: 123
  comm_set_litecomm_type(&block, 0);
  comm_set_litecomm_property(&block, 10);
  acceptor_userdata.liteCommType = 0;
  acceptor_userdata.liteCommProperty = 10;
  acceptor_userdata.int8_data_check = 0;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_REQUEST;
  counter = 0;

  counter = comm_set_data_Uint8(&block, 0, -1);
  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_REQUEST);
  cr_assert_eq(counter, 0);

  cr_log_info("Transmit Test Block. Data: %s", comm_to_str(&block));
  comm_send_block(&source, &block, 0);

  // Set the contents: LiteCommType 14, LiteCommProperty: 2000, Int16: 12343
  comm_set_litecomm_type(&block, 14);
  comm_set_litecomm_property(&block, 2000);
  acceptor_userdata.liteCommType = 14;
  acceptor_userdata.liteCommProperty = 2000;
  acceptor_userdata.int16_data_check = 12343;
  acceptor_userdata.whatToCheck = 1;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE;
  counter = 0;

  counter = comm_set_data_Int16(&block, 12343, -1);
  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE);
  cr_assert_eq(counter, 0);

  cr_log_info("Transmit Test Block. Data: %s", comm_to_str(&block));
  comm_send_block(&source, &block, 0);
}

Test(rcore, tiny_reliable, .init = setup, .fini = teardown)
{
  comm_block_t block;
  comm_init_block(&block);

  comm_set_ack(&block, false);
  comm_set_sStart(&block, true);
  comm_set_sEnd(&block, true);
  comm_set_reliable(&block, true);

  cr_assert(comm_is_tinyPacket(&block));

  // Set the contents: LiteCommType 0, LiteCommProperty: 10, Int8: 123
  comm_set_litecomm_type(&block, 0);
  comm_set_litecomm_property(&block, 10);
  acceptor_userdata.liteCommType = 0;
  acceptor_userdata.liteCommProperty = 10;
  acceptor_userdata.int8_data_check = 123;
  acceptor_userdata.whatToCheck = 0;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_UPDATE;
  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UPDATE);
  int16_t counter = 0;

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);

  counter = comm_set_data_Int8(&block, 123, -1);
  cr_assert_eq(counter, 0);

  cr_log_info("Transmit Test Block. Data: %s", comm_to_str(&block));

  // There should be 1 pending ACK in the source stack, because this is a
  // reliable message.
  sourceToTarget.ack_pending_check = 1;
  comm_send_block(&source, &block, 0);

  sourceToTarget.ack_pending_check = -1;

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);
}

Test(rcore, big_reliable, .init = setup, .fini = teardown)
{
  comm_block_t block;
  comm_init_block(&block);

  comm_set_ack(&block, false);
  comm_set_sStart(&block, true);
  comm_set_sEnd(&block, false);
  comm_set_reliable(&block, true);

  cr_assert(!comm_is_tinyPacket(&block));

  // Set the contents: LiteCommType 2, LiteCommProperty: 11, Int64: 1234141412
  comm_set_litecomm_type(&block, 2);
  comm_set_litecomm_property(&block, 11);
  acceptor_userdata.liteCommType = 2;
  acceptor_userdata.liteCommProperty = 11;
  acceptor_userdata.int64_data_check = 1234141412;
  acceptor_userdata.whatToCheck = 3;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_UPDATE;
  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UPDATE);
  int16_t counter = -1;

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);

  while(counter != 0) {
    if(counter == -1) {
      comm_set_sStart(&block, true);
    } else {
      comm_set_sStart(&block, false);
    }
    counter = comm_set_data_Int64(&block, 1234141412, counter);
    if(counter == 0) {
      comm_set_sEnd(&block, true);
    } else {
      comm_set_sEnd(&block, false);
    }
    cr_log_warn("Transmit Test Block. Data: %s", comm_to_str(&block));
    cr_assert_eq(comm_send_block(&source, &block, 0), LRT_RCORE_OK);
  }
  cr_assert_eq(counter, 0);

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);

  // Do the same again to test repeated transmissions.
  counter = -1;
  while(counter != 0) {
    if(counter == -1) {
      comm_set_sStart(&block, true);
    } else {
      comm_set_sStart(&block, false);
    }
    counter = comm_set_data_Int64(&block, 1234141412, counter);
    if(counter == 0) {
      comm_set_sEnd(&block, true);
    } else {
      comm_set_sEnd(&block, false);
    }
    cr_log_warn("Transmit Test Block. Data: %s", comm_to_str(&block));
    cr_assert_eq(comm_send_block(&source, &block, 0), LRT_RCORE_OK);
  }
  cr_assert_eq(counter, 0);

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);
}

Test(rcore, big_reliable_reversed, .init = setup, .fini = teardown)
{
  comm_block_t block;
  comm_block_t blocks[4];
  comm_init_block(&block);
  for(size_t i = 0; i < 4; ++i) {
    comm_init_block(&blocks[i]);
  }

  comm_set_ack(&block, false);
  comm_set_sStart(&block, true);
  comm_set_sEnd(&block, false);
  comm_set_reliable(&block, true);

  cr_assert(!comm_is_tinyPacket(&block));

  // Set the contents: LiteCommType 3, LiteCommProperty: 11, Int64: 1234141413
  comm_set_litecomm_type(&block, 3);
  comm_set_litecomm_property(&block, 11);
  acceptor_userdata.liteCommType = 3;
  acceptor_userdata.liteCommProperty = 11;
  acceptor_userdata.int64_data_check = 1234141413;
  acceptor_userdata.whatToCheck = 3;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_UPDATE;
  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UPDATE);
  int16_t counter = -1;

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);

  size_t i = 0;

  while(counter != 0) {
    if(counter == -1) {
      comm_set_sStart(&block, true);
    } else {
      comm_set_sStart(&block, false);
    }
    counter = comm_set_data_Int64(&block, 1234141413, counter);
    if(counter == 0) {
      comm_set_sEnd(&block, true);
    } else {
      comm_set_sEnd(&block, false);
    }
    cr_log_warn("Save Test Block for later transit. Data: %s",
                comm_to_str(&block));

    memcpy(blocks[3 - i].data, block.data, iBLOCKSIZE);
    ++i;
  }

  // Transit in reversed order.
  for(size_t i = 0; i < 4; ++i) {
    // Do the sending manually to circumvent automatic sequence number
    // assignments.
    comm_set_sequence_number(&blocks[i], 3 - i);
    comm_ack_stack_insert(&source.ack_stack, &blocks[i]);
    source.transmit(blocks[i].data, source.transmit_userdata, iBLOCKSIZE);
  }

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);
}

Test(rcore, big_reliable_mixed, .init = setup, .fini = teardown)
{
  comm_block_t block;
  comm_block_t blocks[4];
  size_t indices[4] = { 0, 1, 3, 2 };
  comm_init_block(&block);
  for(size_t i = 0; i < 4; ++i) {
    comm_init_block(&blocks[i]);
  }

  comm_set_ack(&block, false);
  comm_set_sStart(&block, true);
  comm_set_sEnd(&block, false);
  comm_set_reliable(&block, true);

  cr_assert(!comm_is_tinyPacket(&block));

  // Set the contents: LiteCommType 3, LiteCommProperty: 11, Int64: 1234141413
  comm_set_litecomm_type(&block, 3);
  comm_set_litecomm_property(&block, 11);
  acceptor_userdata.liteCommType = 3;
  acceptor_userdata.liteCommProperty = 11;
  acceptor_userdata.int64_data_check = 1234141413;
  acceptor_userdata.whatToCheck = 3;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_UPDATE;
  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UPDATE);
  int16_t counter = -1;

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);

  size_t i = 0;

  while(counter != 0) {
    if(counter == -1) {
      comm_set_sStart(&block, true);
    } else {
      comm_set_sStart(&block, false);
    }
    counter = comm_set_data_Int64(&block, 1234141413, counter);
    if(counter == 0) {
      comm_set_sEnd(&block, true);
    } else {
      comm_set_sEnd(&block, false);
    }
    cr_log_warn("Save Test Block for later transit. Data: %s",
                comm_to_str(&block));

    memcpy(blocks[indices[i]].data, block.data, iBLOCKSIZE);
    ++i;
  }

  // Transit in reversed order.
  for(size_t i = 0; i < 4; ++i) {
    // Do the sending manually to circumvent automatic sequence number
    // assignments.
    comm_set_sequence_number(&blocks[i], indices[i]);
    comm_ack_stack_insert(&source.ack_stack, &blocks[i]);
    source.transmit(blocks[i].data, source.transmit_userdata, iBLOCKSIZE);
  }

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);
}

Test(rcore,
     big_reliable_mixed_counter_wrapping,
     .init = setup,
     .fini = teardown)
{
  comm_block_t block;
  comm_block_t blocks[4];
  size_t indices[4] = { 0, 1, 3, 2 };
  comm_init_block(&block);
  for(size_t i = 0; i < 4; ++i) {
    comm_init_block(&blocks[i]);
  }

  comm_set_ack(&block, false);
  comm_set_sStart(&block, true);
  comm_set_sEnd(&block, false);
  comm_set_reliable(&block, true);

  cr_assert(!comm_is_tinyPacket(&block));

  // Set the contents: LiteCommType 3, LiteCommProperty: 11, Int64: 1234141413
  comm_set_litecomm_type(&block, 3);
  comm_set_litecomm_property(&block, 11);
  acceptor_userdata.liteCommType = 3;
  acceptor_userdata.liteCommProperty = 11;
  acceptor_userdata.int64_data_check = 1234141413;
  acceptor_userdata.whatToCheck = 3;
  acceptor_userdata.messageType = LRT_RCP_MESSAGE_TYPE_UPDATE;
  comm_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UPDATE);
  int16_t counter = -1;

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);

  size_t i = 0;

  while(counter != 0) {
    if(counter == -1) {
      comm_set_sStart(&block, true);
    } else {
      comm_set_sStart(&block, false);
    }
    counter = comm_set_data_Int64(&block, 1234141413, counter);
    if(counter == 0) {
      comm_set_sEnd(&block, true);
    } else {
      comm_set_sEnd(&block, false);
    }
    cr_log_warn("Save Test Block for later transit. Data: %s",
                comm_to_str(&block));

    memcpy(blocks[indices[i]].data, block.data, iBLOCKSIZE);
    ++i;
  }

  // Transit in reversed order.
  for(size_t i = 0; i < 4; ++i) {
    // Do the sending manually to circumvent automatic sequence number
    // assignments.
    comm_set_sequence_number(&blocks[i], (indices[i] + 62) & 0b00111111u);
    comm_ack_stack_insert(&source.ack_stack, &blocks[i]);
    source.transmit(blocks[i].data, source.transmit_userdata, iBLOCKSIZE);
  }

  cr_assert_eq(comm_ack_stack_count_pending(&source.ack_stack), 0);
}
