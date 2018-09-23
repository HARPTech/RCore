#include "../include/RCore/librsp/flags.h"
#include <catch.hpp>

TEST_CASE("Setting and getting flags", "[rsp]")
{
  lrt_rbp_message_t message;
  lrt_rbp_message_init(&message, 14, LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8);

  REQUIRE(message._memory >= 14);
  REQUIRE(message.length == 0);

  SECTION("True and False")
  {
    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_ACK));

    rcomm_message_set_flag(&message, LRT_LIBRSP_ACK, true);
    REQUIRE(rcomm_message_has_flag(&message, LRT_LIBRSP_ACK));

    rcomm_message_set_flag(&message, LRT_LIBRSP_ACK, false);
    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_ACK));
  }

  SECTION("Interleaved") {
    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_ACK));
    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_RELIABLE));
    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_STREAM_END));
    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_STREAM_START));
    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_STREAM_TINY));

    rcomm_message_set_flag(&message, LRT_LIBRSP_RELIABLE, true);
    rcomm_message_set_flag(&message, LRT_LIBRSP_ACK, false);
    rcomm_message_set_flag(&message, LRT_LIBRSP_STREAM_END, true);

    REQUIRE(!rcomm_message_has_flag(&message, LRT_LIBRSP_STREAM_TINY));
    REQUIRE(rcomm_message_has_flag(&message, LRT_LIBRSP_STREAM_END));

    rcomm_message_set_flag(&message, LRT_LIBRSP_STREAM_START, true);

    REQUIRE(rcomm_message_has_flag(&message, LRT_LIBRSP_STREAM_TINY));
    REQUIRE(rcomm_message_has_flag(&message, LRT_LIBRSP_STREAM_END));
  }

  lrt_rbp_message_free_internal(&message);
}
