#include <criterion/criterion.h>
#include <criterion/logging.h>

#define LRT_RCORE_DEBUG
#include <RCore/librbp/block.h>

LRT_LIBRBP_BLOCK_STRUCT(test, 8, LRT_LIBRBP_BLOCK_STRUCT_DEBUG_FUNCTIONS)

Test(block, set_data)
{
  test_block_t block;
  test_init(&block);
  test_set_data(&block, 0, 10);
  test_set_data(&block, 1, 20);
  test_set_data(&block, 2, 42);

  // Debug output of the contents.
  cr_log_warn("Block content: %s", test_to_str(&block));

  cr_assert(test_get_data(&block, 0) == 10);
  cr_assert(test_get_data(&block, 1) == 20);
  cr_assert(test_get_data(&block, 2) == 42);
}

Test(block, alternating)
{
  test_block_t block;
  test_init(&block);

  // Brute-Force Testing
  for(size_t i = 0; i < 8 - 1; ++i) {
    if(i % 2 == 0)
      test_set_block_data(&block, i, 0x0F);
    else
      test_set_block_data(&block, i, 0xF0);
  }
  for(size_t i = 0; i < 8 - 1; ++i) {
    if(i % 2 == 0)
      cr_assert(test_get_block_data(&block, i) == 0x0F);
    else
      cr_assert(test_get_block_data(&block, i) == 0xF0);
  }
}
