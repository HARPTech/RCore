#include "../include/RCore/defaults.h"
#include "../include/RCore/rcomm.h"
#include "../include/RCore/sequence_stack.h"
#include <catch.hpp>

TEST_CASE("Initialising and using sequence_stack", "[rcore]")
{
  lrt_rcore_sequence_stack_t* stack = lrt_rcore_sequence_stack_init(10, 10);

  REQUIRE(stack != NULL);

  lrt_rcore_sequence_stack_free(stack);
}
