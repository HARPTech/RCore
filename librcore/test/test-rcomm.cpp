#include "../include/RCore/defaults.h"
#include "../include/RCore/rcomm.h"
#include <catch.hpp>

TEST_CASE("Initialising and using rcomm", "[rcore]")
{
  rcomm_handle_t* handle =
    rcomm_handle_create_from_config(lrt_rcore_rcomm_universal_defaults);

  REQUIRE(handle != NULL);

  rcomm_free(handle);
}
