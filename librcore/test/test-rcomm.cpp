#include "../include/RCore/defaults.h"
#include "../include/RCore/rcomm.h"
#include "../include/RCore/util.hpp"
#include <bitset>
#include <sstream>
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

  // Turn off CRC checking for all tests.
  lrt_rbp_message_set_config(
    &handle->incoming_message, LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8, false);

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

static std::string
convertUint8ArrayToString(const uint8_t* data, size_t bytes)
{
  std::stringstream out;
  for(size_t i = 0; i < bytes; ++i) {
    std::bitset<8> bits(data[i]);
    out << bits << " ";
  }
  return out.str();
}

TEST_CASE("Send and receive over RComm with CRC checking", "[rcore]")
{
  lrt::RCore::RCommHandlePtr handle1 = lrt::RCore::CreateRCommHandlePtr();
  lrt::RCore::RCommHandlePtr handle2 = lrt::RCore::CreateRCommHandlePtr();

  rcomm_set_transmit_cb(
    handle1.get(),
    [](const uint8_t* data, void* userdata, size_t bytes) {
      // Log sent bytes and sizes.
      INFO("Sending " << bytes << " bytes from handle1 to handle2: "
                      << convertUint8ArrayToString(data, bytes));

      rcomm_handle_t* handle = static_cast<rcomm_handle_t*>(userdata);
      lrt_rcore_event_t status = rcomm_parse_bytes(handle, data, bytes);
      REQUIRE(status == LRT_RCORE_OK);
      return status;
    },
    static_cast<void*>(handle2.get()));
  rcomm_set_transmit_cb(
    handle2.get(),
    [](const uint8_t* data, void* userdata, size_t bytes) {
      rcomm_handle_t* handle = static_cast<rcomm_handle_t*>(userdata);
      // Log sent bytes and sizes.
      INFO("Sending " << bytes << " bytes from handle2 to handle1: "
                      << convertUint8ArrayToString(data, bytes));

      lrt_rcore_event_t status = rcomm_parse_bytes(handle, data, bytes);
      REQUIRE(status == LRT_RCORE_OK);
      return status;
    },
    static_cast<void*>(handle1.get()));

  rcomm_set_accept_cb(
    handle1.get(),
    [](lrt_rbp_message_t* message, void* userdata) { return LRT_RCORE_OK; },
    nullptr);
  rcomm_set_accept_cb(
    handle2.get(),
    [](lrt_rbp_message_t* message, void* userdata) { return LRT_RCORE_OK; },
    nullptr);

  REQUIRE(handle1);
  REQUIRE(handle2);

  REQUIRE(rcomm_send_ctrl(
            handle1.get(), 0, 10, LRT_RCP_MESSAGE_TYPE_SUBSCRIBE, true) ==
          LRT_RCORE_OK);
  REQUIRE(rcomm_send_ctrl(
            handle1.get(), 0, 10, LRT_RCP_MESSAGE_TYPE_SUBSCRIBE, false) ==
          LRT_RCORE_OK);
}
