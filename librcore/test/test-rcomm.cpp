#include "../include/RCore/defaults.h"
#include "../include/RCore/rcomm.h"
#include <catch.hpp>

TEST_CASE("Parsing bytes with rcomm", "[rcore]")
{
  rcomm_handle_t* handle =
    rcomm_handle_create_from_config(lrt_rcore_rcomm_universal_defaults);

  rcomm_set_transmit_cb(handle,
                        [](const uint8_t* data, void* userdata, size_t bytes) {
                          return LRT_RCORE_OK;
                        },
                        nullptr);
  rcomm_set_accept_cb(
    handle,
    [](lrt_rbp_message_t* message, void* userdata) { return LRT_RCORE_OK; },
    nullptr);

  REQUIRE(handle != NULL);

  SECTION("Unreliable tiny message")
  {
    uint8_t data[] = { 0b10011000, 0x00, 0x00,       0x00,       0x00, 0x00,
                       0x00,       0x00, 0b10000000, 0b01010101, 0x00, 0x00 };

    REQUIRE(rcomm_parse_bytes(handle, data, 10) == LRT_RCORE_OK);

    REQUIRE(handle->incoming_message.length == 7);
    REQUIRE(handle->incoming_message.data[0] == 0b00110000);

    REQUIRE(rcomm_message_has_flag(&handle->incoming_message,
                                   LRT_LIBRSP_STREAM_START));
    REQUIRE(
      rcomm_message_has_flag(&handle->incoming_message, LRT_LIBRSP_STREAM_END));
    REQUIRE(rcomm_message_has_flag(&handle->incoming_message,
                                   LRT_LIBRSP_STREAM_TINY));
    REQUIRE(
      !rcomm_message_has_flag(&handle->incoming_message, LRT_LIBRSP_RELIABLE));

    REQUIRE(handle->incoming_buffer[0] == 0b10000000);
    REQUIRE(handle->incoming_buffer[1] == 0b01010101);
    REQUIRE(handle->incoming_buffer_size == 2);
  }
  SECTION("Reliable stream message")
  {
    uint8_t data[] = { 0b10110000, 0x00, 0x00,       0x00,       0x00, 0x00,
                       0x00,       0x00, 0b10000000, 0b01010101, 0x00, 0x00 };

    REQUIRE(
      lrt_rcore_sequence_stack_get_entries_in_use(handle->sequence_stack) == 0);
    REQUIRE(lrt_rcore_sequence_stack_get_messages_in_use(
              handle->sequence_stack) == 0);

    REQUIRE(rcomm_parse_bytes(handle, data, 10) == LRT_RCORE_OK);

    REQUIRE(handle->incoming_message.length == 7);

    REQUIRE(handle->incoming_buffer[0] == 0b10000000);
    REQUIRE(handle->incoming_buffer[1] == 0b01010101);
    REQUIRE(handle->incoming_buffer_size == 2);

    REQUIRE(
      lrt_rcore_sequence_stack_get_entries_in_use(handle->sequence_stack) == 1);
    REQUIRE(lrt_rcore_sequence_stack_get_messages_in_use(
              handle->sequence_stack) == 0);
  }

  SECTION("Reliable stream message end")
  {
    uint8_t data[] = { 0b10101000, 0x00, 0x00,       0x00,       0x00, 0x00,
                       0x00,       0x00, 0b10000000, 0b01010101, 0x00, 0x00 };

    REQUIRE(
      lrt_rcore_sequence_stack_get_entries_in_use(handle->sequence_stack) == 0);
    REQUIRE(lrt_rcore_sequence_stack_get_messages_in_use(
              handle->sequence_stack) == 0);

    REQUIRE(rcomm_parse_bytes(handle, data, 10) == LRT_RCORE_OK);

    REQUIRE(handle->incoming_message.length == 7);
    REQUIRE(handle->incoming_message.data[0] == 0b01010000);

    REQUIRE(handle->incoming_buffer[0] == 0b10000000);
    REQUIRE(handle->incoming_buffer[1] == 0b01010101);
    REQUIRE(handle->incoming_buffer_size == 2);

    REQUIRE(
      lrt_rcore_sequence_stack_get_entries_in_use(handle->sequence_stack) == 1);
    REQUIRE(lrt_rcore_sequence_stack_get_messages_in_use(
              handle->sequence_stack) == 1);
  }

  rcomm_free(handle);
}

TEST_CASE("Send and receive over RComm", "[rcore]") {
  rcomm_handle_t* handle =
    rcomm_handle_create_from_config(lrt_rcore_rcomm_universal_defaults);

  rcomm_set_transmit_cb(handle,
                        [](const uint8_t* data, void* userdata, size_t bytes) {
                          return LRT_RCORE_OK;
                        },
                        nullptr);
  rcomm_set_accept_cb(
    handle,
    [](lrt_rbp_message_t* message, void* userdata) { return LRT_RCORE_OK; },
    nullptr);

  REQUIRE(handle != NULL);

  rcomm_free(handle);
}
