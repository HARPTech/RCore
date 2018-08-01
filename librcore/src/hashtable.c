#include "../include/RCore/internal/hashtable.h"

LRT_LIBRCORE_HASHTABLE_KEY_TYPE
lrt__hashtable_key_from_property(uint8_t type, uint16_t property)
{
  union lrt__hashtable_key key = { .hashKey = { .type = type,
                                                .property = property } };
  return key.intKey;
}

struct lrt__hashtable_key_struct
lrt__hashtable_property_from_key(LRT_LIBRCORE_HASHTABLE_KEY_TYPE key)
{
  union lrt__hashtable_key keyUnion = { .intKey = key };
  return keyUnion.hashKey;
}
