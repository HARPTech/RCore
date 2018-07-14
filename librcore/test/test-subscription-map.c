#define LRT_RCORE_DEBUG
#include "../include/RCore/subscription-map.h"
#include <assert.h>
#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <signal.h>
#include <stdint.h>

lrt_rcore_subscription_map_t* map = 0;

const size_t max_subscriptions = 20;

static void
setup(void)
{
  map = lrt_rcore_subscription_map_init(max_subscriptions);
}
static void
teardown(void)
{
  lrt_rcore_subscription_map_free(map);
}

Test(rcore,
     subscription_map_invalid_subscriber,
     .init = setup,
     .fini = teardown,
     .signal = SIGABRT)
{
  lrt_rcore_subscription_map_set_subscribed(map, 10, 1231, 5, true);
}
Test(rcore,
     subscription_map_invalid_type,
     .init = setup,
     .fini = teardown,
     .signal = SIGABRT)
{
  lrt_rcore_subscription_map_set_subscribed(map, 16, 1231, 0, true);
}
Test(rcore,
     subscription_map_invalid_property,
     .init = setup,
     .fini = teardown,
     .signal = SIGABRT)
{
  lrt_rcore_subscription_map_set_subscribed(map, 1, 12301, 0, true);
}
Test(rcore, subscription_map_set_and_query, .init = setup, .fini = teardown)
{
  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 0, 0, 0));

  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 10, 10, 0, true),
               LRT_RCORE_OK);
  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 10, 10, 1, true),
               LRT_RCORE_OK);

  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 10, 11, 0, true),
               LRT_RCORE_OK);
  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 10, 11, 1, true),
               LRT_RCORE_OK);
  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 10, 10, 3, true),
               LRT_RCORE_OK);

  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 0, 0, 0));

  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 0));
  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 10, 11, 0));
  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 10, 11, 3));
  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 3));

  lrt_rcore_subscription_map_set_subscribed(map, 10, 10, 1, false);

  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 0));
  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 1));
  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 2));
}

Test(rcore,
     subscription_map_too_many_subscriptions,
     .init = setup,
     .fini = teardown)
{
  // Fill the subscriptions up.
  for(size_t i = 0; i < max_subscriptions; ++i) {
    for(size_t j = 0; j < 4; ++j) {
      cr_assert_eq(
        lrt_rcore_subscription_map_set_subscribed(map, 1, i, j, true),
        LRT_RCORE_OK);
    }
  }

  // The next subscription should be one too much and trigger an error.
  for(size_t j = 0; j < 4; ++j) {
    cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 2, 1, j, true),
                 LRT_RCORE_MAXIMUM_SUBSCRIPTIONS);
  }

  // But if one is unsubscribed, the next subscription should work again.
  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 1, 1, 1, false),
               LRT_RCORE_OK);

  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 2, 1, 1, true),
               LRT_RCORE_OK);

  // The other ones should still not be possible.
  cr_assert_eq(lrt_rcore_subscription_map_set_subscribed(map, 2, 1, 0, true),
               LRT_RCORE_MAXIMUM_SUBSCRIPTIONS);
}

Test(rcore, subscription_map_request, .init = setup, .fini = teardown)
{
  cr_assert_eq(lrt_rcore_subscription_map_request(map, 10, 10, 1),
               LRT_RCORE_OK);

  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 0));
  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 1));
  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 0));

  // Asking for the subscription again should give unsubscribed.
  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 1));
}

Test(rcore, subscription_map_subscribe_to_all, .init = setup, .fini = teardown)
{
  lrt_rcore_subscription_map_subscribe_to_all(map, 1, true);
  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 10, 10, 1));
  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 10, 20, 1));
  cr_assert(lrt_rcore_subscription_map_is_subscribed(map, 3, 20, 1));
  cr_assert(!lrt_rcore_subscription_map_is_subscribed(map, 3, 20, 0));
}
