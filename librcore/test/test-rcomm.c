#define LRT_RCORE_DEBUG
#include "../include/RCore/rcomm.h"
#include <RCore/librsp/stream_message.h>
#include <criterion/criterion.h>
#include <criterion/logging.h>

LRT_RCORE_RCOMM_DEFINE_PROTOCOL(test, 8u, LRT_LIBRSP_STREAM_MESSAGE, 3u, 8u, 64u)

Test(block, set_data) {}