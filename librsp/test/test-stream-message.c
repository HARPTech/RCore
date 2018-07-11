#include <criterion/criterion.h>
#include <criterion/logging.h>

#define LRT_RCORE_DEBUG
#include <RCore/librbp/block.h>

#define iBLOCKSIZE 8u

LRT_LIBRBP_BLOCK_STRUCT(test, iBLOCKSIZE, LRT_LIBRSP_STREAM_MESSAGE)

void
test_litecomm(test_block_t* block);

void
test_data_various_contents(test_block_t* block)
{
  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    if(i % 2 == 0) {
      test_set_data(block, i, 0xF0);
    } else {
      test_set_data(block, i, 0x0F);
    }
  }

  // Inserting some litecomm changes should do no harm.
  test_litecomm(block);

  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    if(i % 2 == 0) {
      cr_assert_eq(test_get_data(block, i), 0xF0);
    } else {
      cr_assert_eq(test_get_data(block, i), 0x0F);
    }
  }

  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    if(i % 2 == 0) {
      test_set_data(block, i, 0x0F);
    } else {
      test_set_data(block, i, 0xF0);
    }
  }
  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    if(i % 2 == 0) {
      cr_assert_eq(test_get_data(block, i), 0x0F);
    } else {
      cr_assert_eq(test_get_data(block, i), 0xF0);
    }
  }

  test_litecomm(block);

  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    test_set_data(block, i, 42);
  }

  test_litecomm(block);

  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    cr_assert_eq(test_get_data(block, i), 42);
  }

  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    test_set_data(block, i, 0xFF);
  }
  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    cr_assert_eq(test_get_data(block, i), 0xFF);
  }

  test_litecomm(block);

  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    test_set_data(block, i, 0);
  }
  for(size_t i = 0; i < test_get_data_size(block); ++i) {
    cr_assert_eq(test_get_data(block, i), 0);
  }
}

void
test_litecomm(test_block_t* block)
{
  for(uint8_t i = 0; i < 0x0F; ++i) {
    test_set_litecomm_type(block, i);
    test_set_litecomm_property(block, ((uint16_t)i) * 20);
    cr_assert_eq(test_get_litecomm_type(block), i);
    cr_assert_eq(test_get_litecomm_property(block), ((uint16_t)i) * 20);
  }
}

Test(stream_message, set_get_data)
{
  test_block_t block;
  test_init_block(&block);
  test_set_data(&block, 0, 10);
  test_set_data(&block, 1, 20);
  test_set_data(&block, 2, 42);

  cr_assert(test_get_data(&block, 0) == 10);
  cr_assert(test_get_data(&block, 1) == 20);
  cr_assert(test_get_data(&block, 2) == 42);
}

Test(stream_message, message_types)
{
  test_block_t block;
  test_init_block(&block);

  test_set_ack(&block, true);
  test_set_reliable(&block, false);
  test_set_sStart(&block, false);
  test_set_sEnd(&block, true);

  cr_assert(test_is_ack(&block) == true);
  cr_assert(test_is_reliable(&block) == false);
  cr_assert(test_is_sStart(&block) == false);
  cr_assert(test_is_sEnd(&block) == true);
  cr_assert(test_is_iPacket(&block) == false);
  cr_assert(test_is_tinyPacket(&block) == false);

  test_set_sStart(&block, true);
  cr_assert(test_is_tinyPacket(&block) == true);

  test_set_sStart(&block, false);
  test_set_sEnd(&block, false);
  cr_assert(test_is_tinyPacket(&block) == false);
  cr_assert(test_is_iPacket(&block) == true);
}

Test(stream_message, litecomm)
{
  test_block_t block;
  test_init_block(&block);

  test_set_ack(&block, false);
  test_set_reliable(&block, true);
  test_set_sStart(&block, true);
  test_set_sEnd(&block, false);

  // Which makes this packet a reliable stream start packet.
  // Data size should be data size - 4, so 3.
  cr_assert_eq(test_get_data_size(&block), (iBLOCKSIZE - (iBLOCKSIZE / 8) - 4));

  // So setting data should also work from indices 0 to 2.
  test_data_various_contents(&block);

  // If the package becomes unreliable, the data size becomes 4 because of the
  // missing sequence numbers.
  test_set_reliable(&block, false);
  cr_assert_eq(test_get_data_size(&block), (iBLOCKSIZE - (iBLOCKSIZE / 8) - 3));

  // LiteComm should also work correctly when running on unreliable data.
  test_litecomm(&block);

  // So setting data should also work from indices 0 to 3.
  test_data_various_contents(&block);

  // LiteComm should also work correctly after setting data.
  test_litecomm(&block);

  // Now, the sequence numbers should not be possible.
  // But if the reliability bit is set again, the sequence numbers can be tested
  // too.
  test_set_reliable(&block, true);
  test_set_sequence_number(&block, 60);
  test_set_next_sequence_number(&block, 61);
  cr_assert_eq(test_get_sequence_number(&block), 60);
  cr_assert_eq(test_get_next_sequence_number(&block), 61);
  cr_log_warn("Block with sequence numbers: %s", test_to_str(&block));
  test_set_sequence_number(&block, 0);
  test_set_next_sequence_number(&block, 1);
  cr_assert_eq(test_get_sequence_number(&block), 0);
  cr_assert_eq(test_get_next_sequence_number(&block), 1);

  // LiteComm Types should be valid (for range [0,15])
  test_litecomm(&block);
}
