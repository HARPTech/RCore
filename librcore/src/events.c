#include "../include/RCore/events.h"

const char* lrt_rcore_event_names[LRT_RCORE_EVENT_COUNT] = {
  "No new message",
  "Ok",
  "Invalid Block",
  "Stack Full",
  "[ACK Stack] Full map",
  "[ACK Stack] Full queue",
  "[ACK Stack] No entry found",
  "Stack depth exhausted",
  "[Subscription Map] Maximum subscriptions reached",
  "[Subscription Map] Put error",
  "Transmit error",
  "Data left",
  "Alloc failed",
  "[Sequence Stack] Internal error",
  "[Generic] Acceptor error",
  "[Generic] Transmitter error",
  "Not accepted",
  "Not subscribed",
  "[Transmit Buffer] Invalid entry",
  "CRC mismatch",

  "[Block] Too short",
  "[Block] Not dividable by 8",
  "[Block] No start bit",
  "[Block] Start bit inside message",
};
