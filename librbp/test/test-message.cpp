#include "../include/RCore/librbp/message.h"
#include <catch.hpp>

TEST_CASE("Block initialisation", "[rbp]")
{
  lrt_rbp_message_t message;
  lrt_rbp_message_init(
    &message, lrt_rbp_message_length_from_buffer_length(16), 0);

  REQUIRE(message.data != nullptr);
  REQUIRE(message.length == 0);
  REQUIRE(message._memory == 14);

  lrt_rbp_message_free_internal(&message);
}

TEST_CASE("Encode and decode message blocks", "[rbp]")
{
  lrt_rbp_message_t message;
  lrt_rbp_message_init(
    &message, lrt_rbp_message_length_from_buffer_length(16), 0);

  uint8_t data_model[] = { 1u, 15u, 214u, 128u, 0u, 10u };

  // Resize the message to a fitting size and fill it with the provided data.
  lrt_rbp_message_resize(&message, 6);

  for(size_t i = 0; i < 6; ++i)
    message.data[i] = data_model[i];

  // Not initialised.
  const size_t buffer_length = 16;
  uint8_t buffer[buffer_length];
  auto status = lrt_rbp_encode_message(&message, buffer, buffer_length);
  REQUIRE(status == LRT_RCORE_OK);

  // Reset the message.
  for(size_t i = 0; i < 6; ++i) {
    message.data[i] = 0;
    REQUIRE((int)message.data[i] == 0);
  }

  // Decode the message again.
  status = lrt_rbp_decode_message(&message, buffer, buffer_length);
  REQUIRE(status == LRT_RCORE_OK);

  // Check the internal message length.
  REQUIRE(message.length == 14);

  for(size_t i = 0; i < 6; ++i)
    REQUIRE((int)message.data[i] == (int)data_model[i]);

  lrt_rbp_message_free_internal(&message);
}

TEST_CASE("Check block validity", "[rbp]")
{
  uint8_t block[16] = {
    0b10000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  REQUIRE(lrt_rbp_is_block_valid(block, 16) == LRT_RCORE_OK);

  block[2] = 0xFF;

  REQUIRE(lrt_rbp_is_block_valid(block, 16) ==
          LRT_RCORE_BLOCK_START_BIT_INSIDE_MESSAGE);

  block[2] = 0;

  REQUIRE(lrt_rbp_is_block_valid(block, 16) == LRT_RCORE_OK);
  REQUIRE(lrt_rbp_is_block_valid(block, 15) ==
          LRT_RCORE_BLOCK_NOT_DIVIDABLE_BY_8);
  REQUIRE(lrt_rbp_is_block_valid(block, 8) == LRT_RCORE_OK);
}
