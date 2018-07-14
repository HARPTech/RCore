#ifndef LRT_LIBRCORE_SUBSCRIPTION_MAP_H
#define LRT_LIBRCORE_SUBSCRIPTION_MAP_H

#include "events.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

  typedef struct lrt_rcore_subscription_map_t lrt_rcore_subscription_map_t;

  lrt_rcore_subscription_map_t* lrt_rcore_subscription_map_init(
    size_t max_subscriptions);
  void lrt_rcore_subscription_map_free(lrt_rcore_subscription_map_t* map);

  bool lrt_rcore_subscription_map_is_subscribed(
    lrt_rcore_subscription_map_t* map,
    uint8_t type,
    uint16_t property,
    uint8_t subscriber);

  lrt_rcore_event_t lrt_rcore_subscription_map_set_subscribed(
    lrt_rcore_subscription_map_t* map,
    uint8_t type,
    uint16_t property,
    uint8_t subscriber,
    bool subscribed);

  lrt_rcore_event_t lrt_rcore_subscription_map_request(
    lrt_rcore_subscription_map_t* map,
    uint8_t type,
    uint16_t property,
    uint8_t subscriber);

  size_t lrt_rcore_subscription_map_max_subscriptions(
    lrt_rcore_subscription_map_t* map);

  void lrt_rcore_subscription_map_subscribe_to_all(
    lrt_rcore_subscription_map_t* map,
    uint8_t subscriber,
    bool subscribe_to_all);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
