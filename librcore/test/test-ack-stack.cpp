#include "../include/RCore/defaults.h"
#include "../include/RCore/rcomm.h"
#include "../include/RCore/ack_stack.h"
#include <catch.hpp>

TEST_CASE("Initialising and using ack_stack", "[rcore]")
{
  lrt_rcore_ack_stack_t *stack = lrt_rcore_ack_stack_init(4, 10, 10, 10);

  REQUIRE(stack != NULL);

  lrt_rcore_ack_stack_free(stack);
}
