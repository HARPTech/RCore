#include "../include/RCore/librsp/litecomm.h"
#include <catch.hpp>

TEST_CASE("Setting and getting litecomm type and property", "[rsp]")
{
  lrt_rbp_message_t message;
  lrt_rbp_message_init(&message, 14, LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8);

  REQUIRE(message._memory >= 14);
  REQUIRE(message.length == 0);

  REQUIRE(rcomm_message_get_litecomm_type(&message) == 0);
  REQUIRE(rcomm_message_get_litecomm_property(&message) == 0);
  REQUIRE(rcomm_message_get_sequence_number(&message) == 0);

  SECTION("Setting and getting standard values")
  {
    rcomm_message_set_litecomm_type(&message, 10);
    rcomm_message_set_litecomm_property(&message, 42);
    REQUIRE(rcomm_message_get_litecomm_type(&message) == 10);
    REQUIRE(rcomm_message_get_litecomm_property(&message) == 42);

    rcomm_message_set_litecomm_property(&message, 100);
    rcomm_message_set_litecomm_type(&message, 4);
    REQUIRE(rcomm_message_get_litecomm_type(&message) == 4);
    REQUIRE(rcomm_message_get_litecomm_property(&message) == 100);

    rcomm_message_set_litecomm_type(&message, 0);
    rcomm_message_set_litecomm_property(&message, 0);
    REQUIRE(rcomm_message_get_litecomm_type(&message) == 0);
    REQUIRE(rcomm_message_get_litecomm_property(&message) == 0);
  }

  SECTION("Setting and getting extreme values")
  {
    rcomm_message_set_litecomm_property(&message, 0x0F0F);
    REQUIRE(rcomm_message_get_litecomm_property(&message) == 0xF0F);
    rcomm_message_set_litecomm_property(&message, 0x0FFF);
    REQUIRE(rcomm_message_get_litecomm_property(&message) == 0xFFF);

    rcomm_message_set_litecomm_type(&message, 12);
    REQUIRE(rcomm_message_get_litecomm_type(&message) == 12);
    rcomm_message_set_litecomm_type(&message, 16);
    REQUIRE(rcomm_message_get_litecomm_type(&message) ==
            0);// Setting 16 overflows, it should be 0.

    rcomm_message_set_litecomm_property(&message, 0x1000);
    REQUIRE(rcomm_message_get_litecomm_property(&message) == 0);
    REQUIRE(rcomm_message_get_litecomm_type(&message) == 0);
  }

  lrt_rbp_message_free_internal(&message);
}
