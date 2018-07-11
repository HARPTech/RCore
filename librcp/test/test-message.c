#include "../include/RCore/librcp/message.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

#define LRT_RCORE_DEBUG
#include <RCore/librbp/block.h>
#include <RCore/librsp/stream_message.h>

LRT_LIBRBP_BLOCK_STRUCT(test, 64u, LRT_LIBRSP_STREAM_MESSAGE)

// Types
LRT_LIBRCP_TYPES(test)

// Test Helper
#define SET_GET_CHECK_DATA_HELPER_ASSERT(                        \
  sPREFIX, sTYPENAME, tTYPE, oBLOCK, VALUE, ASSERT)              \
  {                                                              \
    tTYPE data = 0;                                              \
    for(size_t i = -1, j = -1; i != 0 && j != 0;) {              \
      i = sPREFIX##_set_data_##sTYPENAME(&(oBLOCK), (VALUE), i); \
      cr_log_info("Block: %s", sPREFIX##_to_str(block));         \
      j = sPREFIX##_get_data_##sTYPENAME(&(oBLOCK), &data, j);   \
    }                                                            \
    cr_log_info("Check");                                        \
    ASSERT(data, VALUE);                                         \
  }

#define NORMAL_ASSERT(ACTUAL, EXPECTED) cr_assert_eq(ACTUAL, EXPECTED)

#define FLOAT_ASSERT(ACTUAL, EXPECTED) cr_assert_float_eq(ACTUAL, EXPECTED, 0.1)

#define SET_GET_CHECK_DATA_HELPER(sPREFIX, sTYPENAME, tTYPE, oBLOCK, VALUE) \
  SET_GET_CHECK_DATA_HELPER_ASSERT(                                         \
    sPREFIX, sTYPENAME, tTYPE, oBLOCK, VALUE, NORMAL_ASSERT)

#define SET_GET_CHECK_DATA_HELPER_FLOAT(    \
  sPREFIX, sTYPENAME, tTYPE, oBLOCK, VALUE) \
  SET_GET_CHECK_DATA_HELPER_ASSERT(         \
    sPREFIX, sTYPENAME, tTYPE, oBLOCK, VALUE, FLOAT_ASSERT)

void
test_litecomm_sequence_numbers(test_block_t* block)
{
  for(size_t i = 0; i < 0b00111111u; ++i) {
    test_set_litecomm_sequence_number(block, i);
    cr_assert_eq(test_get_litecomm_sequence_number(block), i);
  }
}

void
test_set_data_types_simple(test_block_t* block)
{
  test_litecomm_sequence_numbers(block);
  SET_GET_CHECK_DATA_HELPER(test, Int8, int8_t, *block, 10);
  SET_GET_CHECK_DATA_HELPER(test, Uint8, uint8_t, *block, 42);

  SET_GET_CHECK_DATA_HELPER(test, Int16, int16_t, *block, -0x0F0F);
  SET_GET_CHECK_DATA_HELPER(test, Int16, int16_t, *block, 0x0FFF);

  SET_GET_CHECK_DATA_HELPER(test, Bool, bool, *block, true);
  test_litecomm_sequence_numbers(block);
  SET_GET_CHECK_DATA_HELPER(test, Bool, bool, *block, false);

  SET_GET_CHECK_DATA_HELPER(test, Uint16, uint16_t, *block, 0x0FFF);
  SET_GET_CHECK_DATA_HELPER(test, Uint16, uint16_t, *block, 0xFFFF);
  SET_GET_CHECK_DATA_HELPER(test, Uint16, uint16_t, *block, 0);
}
void
test_set_data_types_complex(test_block_t* block)
{
  SET_GET_CHECK_DATA_HELPER(test, Uint32, uint32_t, *block, 0xFF00FF00);
  SET_GET_CHECK_DATA_HELPER(test, Uint32, uint32_t, *block, 0xFFFFFFFF);

  SET_GET_CHECK_DATA_HELPER(test, Uint64, uint64_t, *block, 0xFF00FF00FFFFFFFF);
  SET_GET_CHECK_DATA_HELPER(test, Uint64, uint64_t, *block, 0xFFFFFFFFFFFFFFFF);
  SET_GET_CHECK_DATA_HELPER(test, Uint64, uint64_t, *block, 0xF0F0F0F0F0F0F0F0);
  SET_GET_CHECK_DATA_HELPER(test, Uint64, uint64_t, *block, 0xFFF0FFF0FFF0FFF0);
  SET_GET_CHECK_DATA_HELPER(test, Uint64, uint64_t, *block, 0x000F00F0FFF0FFA0);

  test_litecomm_sequence_numbers(block);

  SET_GET_CHECK_DATA_HELPER(test, Int64, int64_t, *block, 0x000F00F0FFF0FFA0);
  SET_GET_CHECK_DATA_HELPER(test, Int64, int64_t, *block, 12314134421321332);
  SET_GET_CHECK_DATA_HELPER(test, Int64, int64_t, *block, -12314134421321332);

  // Floats and Doubles.

  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Float, float, *block, 100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Float, float, *block, 100.131);
  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Float, float, *block, -100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Float, float, *block, 124141.131);
  test_litecomm_sequence_numbers(block);

  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Double, double, *block, 100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Double, double, *block, 100.131);
  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Double, double, *block, -100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(test, Double, double, *block, 124141.131);
  SET_GET_CHECK_DATA_HELPER_FLOAT(
    test, Double, double, *block, 12414124121.131);
}

void
test_set_data_types(test_block_t* block)
{
  test_set_data_types_simple(block);
  test_set_data_types_complex(block);
}

Test(message, types)
{
  test_block_t block;
  test_init_block(&block);

  // Test message types.
  test_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UPDATE);
  cr_assert_eq(test_get_litecomm_message_type(&block),
               LRT_RCP_MESSAGE_TYPE_UPDATE);
  test_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_SUBSCRIBE);
  cr_assert_eq(test_get_litecomm_message_type(&block),
               LRT_RCP_MESSAGE_TYPE_SUBSCRIBE);
  test_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE);
  cr_assert_eq(test_get_litecomm_message_type(&block),
               LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE);
  test_set_litecomm_message_type(&block, LRT_RCP_MESSAGE_TYPE_REQUEST);
  cr_assert_eq(test_get_litecomm_message_type(&block),
               LRT_RCP_MESSAGE_TYPE_REQUEST);

  // Unreliable Packets.
  test_set_reliable(&block, false);
  test_set_data_types(&block);

  // Reliable Packets.
  test_set_reliable(&block, true);
  test_set_data_types(&block);
}
