#include "../include/RCore/defaults.h"
#include "../include/RCore/rcomm.h"
#include "../include/RCore/util.hpp"
#include <algorithm>
#include <bitset>
#include <catch.hpp>
#include <random>
#include <sstream>

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

std::default_random_engine generator(time(0));
std::uniform_int_distribution<int> distributionBitAmount(1, 3);

using MsgArray = std::array<uint8_t, 16>;

static void
randomlyMutateArray(MsgArray& arr, size_t times, size_t max_byte)
{
  std::vector<size_t> possibilities(8 * max_byte);
  std::iota(std::begin(possibilities), std::end(possibilities), 0);
  std::random_shuffle(std::begin(possibilities), std::end(possibilities));

  REQUIRE(possibilities.size() == max_byte * 8);

  for(size_t i = 0; i < times; ++i) {
    size_t byte = possibilities[i] / 8;
    size_t bit = possibilities[i] % 8;

    arr[byte] ^= 1u << bit;
  }
  CAPTURE(times);
}

TEST_CASE("Send and receive over RComm with CRC mismatch detection", "[rcore]")
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

      // Randomly change bits in message.
      std::array<uint8_t, 16> arr;
      std::copy(data, data + bytes, std::begin(arr));
      randomlyMutateArray(arr, distributionBitAmount(generator), bytes);

      // Check that the array is different than before.
      bool different = false;
      for(size_t i = 0; i < bytes; ++i) {
        if(arr[i] != data[i]) {
          different = true;
        }
      }
      INFO("(After Transform) Sending "
           << bytes << " bytes from handle1 to handle2: "
           << convertUint8ArrayToString(arr.data(), bytes));
      REQUIRE(different);

      lrt_rcore_event_t status = rcomm_parse_bytes(handle, arr.data(), bytes);
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

      // Randomly change 2 bits in message.
      std::array<uint8_t, 16> arr;
      std::copy(data, data + bytes, std::begin(arr));
      randomlyMutateArray(arr, 2, bytes);

      // Check that the array is different than before.
      bool different = false;
      for(size_t i = 0; i < bytes; ++i) {
        if(arr[i] != data[i]) {
          different = true;
        }
      }
      REQUIRE(different);

      lrt_rcore_event_t status = rcomm_parse_bytes(handle, arr.data(), bytes);
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

  lrt_rbp_message_t message;
  lrt_rbp_message_init(&message, 14, LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8);

  const size_t tries = 100;

  for(size_t i = 0; i < tries; ++i) {

    lrt_rcore_event_t status = LRT_RCORE_OK;

    status = rcomm_send_ctrl(
      handle1.get(), 5, 100, LRT_RCP_MESSAGE_TYPE_SUBSCRIBE, false);

    {
      CAPTURE(status);
      CAPTURE(i);

      REQUIRE((status == LRT_RCORE_CRC_MISMATCH ||
               status == LRT_RCORE_NO_ACK_ENTRY_FOUND ||
               status == LRT_RCORE_BLOCK_NO_START_BIT ||
               status == LRT_RCORE_BLOCK_START_BIT_INSIDE_MESSAGE ||
               status == LRT_RCORE_NO_NEW_MESSAGE));
    }

    lrt_rbp_message_reset_data(&message);

    rcomm_message_set_flag(&message, LRT_LIBRSP_RELIABLE, false);
    rcomm_message_set_flag(&message, LRT_LIBRSP_STREAM_START, true);
    rcomm_message_set_flag(&message, LRT_LIBRSP_STREAM_END, true);
    lrt_librcp_Int16_set_data(&message, 10000);
    rcomm_message_set_litecomm_type(&message, 5);
    rcomm_message_set_litecomm_property(&message, 100);
    rcomm_message_set_sequence_number(&message, i);

    status = rcomm_transmit_message(handle1.get(), &message);

    {
      CAPTURE(status);
      CAPTURE(i);

      REQUIRE((status == LRT_RCORE_CRC_MISMATCH ||
               status == LRT_RCORE_NO_ACK_ENTRY_FOUND ||
               status == LRT_RCORE_BLOCK_NO_START_BIT ||
               status == LRT_RCORE_BLOCK_START_BIT_INSIDE_MESSAGE ||
               status == LRT_RCORE_NO_NEW_MESSAGE));
    }

    lrt_rbp_message_reset_data(&message);
  }

  lrt_rbp_message_free_internal(&message);
}
