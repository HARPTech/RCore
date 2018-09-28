#include "../include/RCore/librcp/message.h"
#include <catch.hpp>

// Test Helper
#define SET_GET_CHECK_DATA_HELPER_ASSERT(                     \
  sTYPENAME, tTYPE, oMESSAGE, VALUE, ASSERT)                  \
  {                                                           \
    tTYPE data = 0;                                           \
    data = VALUE;                                             \
    data = lrt_librcp_##sTYPENAME##_from_data(                \
      lrt_librcp_##sTYPENAME##_to_data(data), sizeof(tTYPE)); \
    ASSERT(data, VALUE);                                      \
    /* Same with the message. */                              \
    data = VALUE;                                             \
    lrt_librcp_##sTYPENAME##_set_data(oMESSAGE, data);        \
    data = lrt_librcp_##sTYPENAME##_get_data(oMESSAGE);       \
    ASSERT(data, VALUE);                                      \
  }

#define NORMAL_ASSERT(ACTUAL, EXPECTED) REQUIRE(ACTUAL == EXPECTED)

#define FLOAT_ASSERT(ACTUAL, EXPECTED) REQUIRE(ACTUAL == Approx(EXPECTED))

#define SET_GET_CHECK_DATA_HELPER(sTYPENAME, tTYPE, oMESSAGE, VALUE) \
  SET_GET_CHECK_DATA_HELPER_ASSERT(                                  \
    sTYPENAME, tTYPE, oMESSAGE, VALUE, NORMAL_ASSERT)

#define SET_GET_CHECK_DATA_HELPER_FLOAT(sTYPENAME, tTYPE, oMESSAGE, VALUE) \
  SET_GET_CHECK_DATA_HELPER_ASSERT(                                        \
    sTYPENAME, tTYPE, oMESSAGE, VALUE, FLOAT_ASSERT)

void
test_set_data_types_simple(lrt_rbp_message_t* message)
{
  SET_GET_CHECK_DATA_HELPER(Int8, int8_t, message, 10);
  SET_GET_CHECK_DATA_HELPER(Uint8, uint8_t, message, 42);

  SET_GET_CHECK_DATA_HELPER(Int16, int16_t, message, -0x0F0F);
  SET_GET_CHECK_DATA_HELPER(Int16, int16_t, message, 0x0FFF);

  SET_GET_CHECK_DATA_HELPER(Bool, bool, message, true);
  SET_GET_CHECK_DATA_HELPER(Bool, bool, message, false);

  SET_GET_CHECK_DATA_HELPER(Uint16, uint16_t, message, 0x0FFF);
  SET_GET_CHECK_DATA_HELPER(Uint16, uint16_t, message, 0xFFFF);
  SET_GET_CHECK_DATA_HELPER(Uint16, uint16_t, message, 0);
}
void
test_set_data_types_complex(lrt_rbp_message_t* message)
{
  SET_GET_CHECK_DATA_HELPER(Uint32, uint32_t, message, 0xFF00FF00);
  SET_GET_CHECK_DATA_HELPER(Uint32, uint32_t, message, 0xFFFFFFFF);

  SET_GET_CHECK_DATA_HELPER(Uint64, uint64_t, message, 0xFF00FF00FFFFFFFF);
  SET_GET_CHECK_DATA_HELPER(Uint64, uint64_t, message, 0xFFFFFFFFFFFFFFFF);
  SET_GET_CHECK_DATA_HELPER(Uint64, uint64_t, message, 0xF0F0F0F0F0F0F0F0);
  SET_GET_CHECK_DATA_HELPER(Uint64, uint64_t, message, 0xFFF0FFF0FFF0FFF0);
  SET_GET_CHECK_DATA_HELPER(Uint64, uint64_t, message, 0x000F00F0FFF0FFA0);

  SET_GET_CHECK_DATA_HELPER(Int64, int64_t, message, 0x000F00F0FFF0FFA0);
  SET_GET_CHECK_DATA_HELPER(Int64, int64_t, message, 12314134421321332);
  SET_GET_CHECK_DATA_HELPER(Int64, int64_t, message, -12314134421321332);

  // Floats and Doubles.

  SET_GET_CHECK_DATA_HELPER_FLOAT(Float, float, message, 100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(Float, float, message, 100.131);
  SET_GET_CHECK_DATA_HELPER_FLOAT(Float, float, message, -100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(Float, float, message, 124141.131);

  SET_GET_CHECK_DATA_HELPER_FLOAT(Double, double, message, 100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(Double, double, message, 100.131);
  SET_GET_CHECK_DATA_HELPER_FLOAT(Double, double, message, -100.0);
  SET_GET_CHECK_DATA_HELPER_FLOAT(Double, double, message, 124141.131);
  SET_GET_CHECK_DATA_HELPER_FLOAT(Double, double, message, 12414124121.131);
}

void
test_set_data_types(lrt_rbp_message_t* message)
{
  test_set_data_types_simple(message);
  test_set_data_types_complex(message);
}

TEST_CASE("Test setting and getting data types into/from messages.", "[rcp]")
{
  lrt_rbp_message_t message;
  lrt_rbp_message_init(&message, 14, 0);

  // Test message types.
  rcomm_set_litecomm_message_type(&message, LRT_RCP_MESSAGE_TYPE_UPDATE);
  REQUIRE(rcomm_get_litecomm_message_type(&message) ==
          LRT_RCP_MESSAGE_TYPE_UPDATE);
  rcomm_set_litecomm_message_type(&message, LRT_RCP_MESSAGE_TYPE_SUBSCRIBE);
  REQUIRE(rcomm_get_litecomm_message_type(&message) ==
          LRT_RCP_MESSAGE_TYPE_SUBSCRIBE);
  rcomm_set_litecomm_message_type(&message, LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE);
  REQUIRE(rcomm_get_litecomm_message_type(&message) ==
          LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE);
  rcomm_set_litecomm_message_type(&message, LRT_RCP_MESSAGE_TYPE_REQUEST);
  REQUIRE(rcomm_get_litecomm_message_type(&message) ==
          LRT_RCP_MESSAGE_TYPE_REQUEST);

  test_set_data_types(&message);
}
