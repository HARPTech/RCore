#include "../include/RCore/librsp/data.h"
#include <catch.hpp>

TEST_CASE("Setting and getting data", "[rsp]")
{
  lrt_rbp_message_t message;
  lrt_rbp_message_init(&message, 14, LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8);

  REQUIRE(message._memory >= 14);
  REQUIRE(message.length == 0);

  // Data size has to be 8, because there is an offset of 5 and an activated
  // CRC8 check, which reduces the 14 length by 1 additional byte at the end.
  REQUIRE(rcomm_message_get_data_size(&message) == 8);

  // Calculate the CRC8.
  REQUIRE(message._memory - rcomm_message_get_data_offset(&message) -
            rcomm_message_get_data_size(&message) ==
          1);

  // Calculate the CRC32.
  message.config = LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC32;
  REQUIRE(message._memory - rcomm_message_get_data_offset(&message) -
            rcomm_message_get_data_size(&message) ==
          4);

  // Reset to CRC8 as per default.
  message.config = LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8;
  REQUIRE(message._memory - rcomm_message_get_data_offset(&message) -
            rcomm_message_get_data_size(&message) ==
          1);

  SECTION("Setting data into the message and checking length")
  {
    uint8_t data[] = { 0, 1, 45, 1, 4 };
    REQUIRE(rcomm_message_insert_data(&message, data, 5, 0) == 5);

    REQUIRE(message.length == 5 + 5 + 1);

    uint8_t encoded[16];

    lrt_rbp_encode_message(&message, encoded, 16);
    lrt_rbp_decode_message(&message, encoded, 16);

    for(size_t i = 0; i < 5; ++i) {
      REQUIRE(message.data[i + rcomm_message_get_data_offset(&message)] ==
              data[i]);
    }
  }

  lrt_rbp_message_free_internal(&message);
}
