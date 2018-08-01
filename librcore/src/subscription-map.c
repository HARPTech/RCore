#include "../include/RCore/internal/hashtable.h"
#include <assert.h>
#include <klib/khash.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* This enum is used to interpret the integers inside the hash map. */
enum hash_value_enum
{
  PROPERTY_UNSUBSCRIBED = 0u,
  PROPERTY_SUBSCRIBED = 1u,
  PROPERTY_SINGLE_USE_SUBSCRIPTION = 2u
};

LRT_LIBRCORE_HASHTABLE_INIT(int16map, uint8_t)

struct lrt_rcore_subscription_map_t
{
  khash_t(int16map) * map;
  size_t subscriptions[4];
  size_t max_subscriptions;
  bool subscribe_to_all[4];
};

#include "../include/RCore/subscription-map.h"

/* The subscription map is initialised in this source file as a khash
 * hash-table. This hash table only uses dynamic memory allocation at the
 * initialisation step and is constant from that point onward. No implementation
 * details are leaked outside of this scope. */

lrt_rcore_subscription_map_t*
lrt_rcore_subscription_map_init(size_t max_subscriptions)
{
  lrt_rcore_subscription_map_t* map =
    calloc(1, sizeof(lrt_rcore_subscription_map_t));

  map->map = kh_init_int16map();
  map->max_subscriptions = max_subscriptions;

  for(size_t i = 0; i < 4; ++i) {
    map->subscriptions[i] = 0;
    map->subscribe_to_all[i] = false;
  }

  // Resize to the maximum number of subscriptions given to this initialisation
  // function.
  kh_resize_int16map(map->map, max_subscriptions);

  return map;
}
void
lrt_rcore_subscription_map_free(lrt_rcore_subscription_map_t* map)
{
  assert(map != 0);
  assert(map->map != 0);

  kh_destroy_int16map(map->map);
  free(map);
}

static enum hash_value_enum
get_enum_from_map(lrt_rcore_subscription_map_t* map,
                  uint8_t type,
                  uint16_t property,
                  uint8_t subscriber)
{
  assert(map != 0);
  assert(map->map != 0);
  assert(type < 16);      // 2^4 is the type limit.
  assert(property < 4096);// 2^12 is the property limit.
  assert(subscriber < 4); // maximum 4 different subscribers supported.

  LRT_LIBRCORE_HASHTABLE_KEY_TYPE key =
    lrt__hashtable_key_from_property(type, property);

  khint_t it = kh_get_int16map(map->map, key);

  if(it == kh_end(map->map)) {
    return PROPERTY_UNSUBSCRIBED;
  }

  uint8_t intVal = kh_val(map->map, it);

  enum hash_value_enum val =
    (enum hash_value_enum)((intVal >> (subscriber * 2u)) & 0b00000011);
  return val;
}
static lrt_rcore_event_t
set_enum_to_map(lrt_rcore_subscription_map_t* map,
                uint8_t type,
                uint16_t property,
                uint8_t subscriber,
                enum hash_value_enum value)
{
  assert(map != 0);
  assert(map->map != 0);
  assert(type < 16);      // 2^4 is the type limit.
  assert(property < 4096);// 2^12 is the property limit.
  assert(subscriber < 4); // maximum 4 different subscribers supported.

  LRT_LIBRCORE_HASHTABLE_KEY_TYPE key =
    lrt__hashtable_key_from_property(type, property);

  uint8_t entry = value << (subscriber * 2u);
  khint_t it = kh_get_int16map(map->map, key);
  if(it == kh_end(map->map)) {
    // Insert the new key.
    int ret = 0;
    it = kh_put_int16map(map->map, key, &ret);
    switch(ret) {
      case -1:
        return LRT_RCORE_SUBSCRIPTION_MAP_PUT_ERROR;
      default:
        break;
    }
  } else {
    // Use the existing value.
    entry = kh_val(map->map, it);
    entry = (entry & ~(0b11u << (subscriber * 2u))) |
            (((uint8_t)value) << (subscriber * 2u));
  }

  switch(value) {
    case PROPERTY_SINGLE_USE_SUBSCRIPTION:
    case PROPERTY_SUBSCRIBED:
      if(map->subscriptions[subscriber] < map->max_subscriptions) {
        ++map->subscriptions[subscriber];
        kh_val(map->map, it) = entry;
      } else {
        return LRT_RCORE_MAXIMUM_SUBSCRIPTIONS;
      }
      break;
    case PROPERTY_UNSUBSCRIBED:
      if(map->subscriptions[subscriber] > 0) {
        --map->subscriptions[subscriber];
      }

      if(entry == 0) {
        kh_del_int16map(map->map, it);
      } else {
        kh_val(map->map, it) = entry;
      }
      break;
  }
  return LRT_RCORE_OK;
}

bool
lrt_rcore_subscription_map_is_subscribed(lrt_rcore_subscription_map_t* map,
                                         uint8_t type,
                                         uint16_t property,
                                         uint8_t subscriber)
{
  assert(map != 0);
  assert(subscriber < 4);

  if(map->subscribe_to_all[subscriber]) {
    return true;
  }

  enum hash_value_enum value =
    get_enum_from_map(map, type, property, subscriber);

  switch(value) {
    case PROPERTY_UNSUBSCRIBED:
      return false;
    case PROPERTY_SUBSCRIBED:
      return true;
    case PROPERTY_SINGLE_USE_SUBSCRIPTION: {
      lrt_rcore_event_t status =
        set_enum_to_map(map, type, property, subscriber, PROPERTY_UNSUBSCRIBED);
      assert(status == LRT_RCORE_OK);
      return true;
    }
    default: {
      // The value must have been corrupted, reset it to 0.
      lrt_rcore_event_t status =
        set_enum_to_map(map, type, property, subscriber, PROPERTY_UNSUBSCRIBED);
      assert(status == LRT_RCORE_OK);
      return false;
    }
  }
}

uint8_t
lrt_rcore_subscription_map_get_subscribers(lrt_rcore_subscription_map_t* map,
                                           uint8_t type,
                                           uint16_t property)
{
  assert(map != 0);
  assert(map->map != 0);
  assert(type < 16);      // 2^4 is the type limit.
  assert(property < 4096);// 2^12 is the property limit.

  LRT_LIBRCORE_HASHTABLE_KEY_TYPE key =
    lrt__hashtable_key_from_property(type, property);

  khint_t it = kh_get_int16map(map->map, key);

  if(it == kh_end(map->map)) {
    return 0;
  }

  uint8_t intVal = kh_val(map->map, it);

  uint8_t subscribers = 0;

  for(size_t i = 0; i < 4; ++i) {
    enum hash_value_enum val =
      (enum hash_value_enum)((intVal >> (i * 2u)) & 0b00000011);

    switch(val) {
      case PROPERTY_UNSUBSCRIBED:
        // Unsubscribed does not need to do anything.
        break;
      case PROPERTY_SUBSCRIBED:
        subscribers |= 1 << i;
        break;
      case PROPERTY_SINGLE_USE_SUBSCRIPTION: {
        lrt_rcore_event_t status = set_enum_to_map(
          map, type, property, i, PROPERTY_UNSUBSCRIBED);
        assert(status == LRT_RCORE_OK);
        subscribers |= 1 << i;
        break;
      }
      default: {
        // The value must have been corrupted, reset it to 0.
        lrt_rcore_event_t status = set_enum_to_map(
          map, type, property, i, PROPERTY_UNSUBSCRIBED);
        assert(status == LRT_RCORE_OK);
        break;
      }
    }
  }
  return subscribers;
}

lrt_rcore_event_t
lrt_rcore_subscription_map_set_subscribed(lrt_rcore_subscription_map_t* map,
                                          uint8_t type,
                                          uint16_t property,
                                          uint8_t subscriber,
                                          bool subscribed)
{
  return set_enum_to_map(map,
                         type,
                         property,
                         subscriber,
                         subscribed ? PROPERTY_SUBSCRIBED
                                    : PROPERTY_UNSUBSCRIBED);
}

lrt_rcore_event_t
lrt_rcore_subscription_map_request(lrt_rcore_subscription_map_t* map,
                                   uint8_t type,
                                   uint16_t property,
                                   uint8_t subscriber)
{
  return set_enum_to_map(
    map, type, property, subscriber, PROPERTY_SINGLE_USE_SUBSCRIPTION);
}

size_t
lrt_rcore_subscription_map_max_subscriptions(lrt_rcore_subscription_map_t* map)
{
  assert(map != 0);
  return map->max_subscriptions;
}

void
lrt_rcore_subscription_map_subscribe_to_all(lrt_rcore_subscription_map_t* map,
                                            uint8_t subscriber,
                                            bool subscribe_to_all)
{
  assert(map != 0);
  assert(subscriber < 4);
  map->subscribe_to_all[subscriber] = subscribe_to_all;
}
