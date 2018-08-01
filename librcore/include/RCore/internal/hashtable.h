#ifndef LRT_LIBRCORE_INTERNAL_HASHTABLE_H
#define LRT_LIBRCORE_INTERNAL_HASHTABLE_H

/* @file
 * Provides a hash table from khash with a predefined key type, describing the
 * type and property of a LiteComm value.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <klib/khash.h>

#define LRT_LIBRCORE_HASHTABLE_KEY_TYPE uint16_t

  /* This structure is used to read values from integer hash keys. */
  struct lrt__hashtable_key_struct
  {
    uint8_t type : 4;
    uint16_t property : 12;
  };
  union lrt__hashtable_key
  {
    LRT_LIBRCORE_HASHTABLE_KEY_TYPE intKey;
    struct lrt__hashtable_key_struct hashKey;
  };

  LRT_LIBRCORE_HASHTABLE_KEY_TYPE lrt__hashtable_key_from_property(
    uint8_t type,
    uint16_t property);

  struct lrt__hashtable_key_struct lrt__hashtable_property_from_key(
    LRT_LIBRCORE_HASHTABLE_KEY_TYPE key);

#define LRT_LIBRCORE_HASHTABLE_INIT(NAME, PAYLOAD_TYPE) \
  KHASH_INIT(NAME,                                      \
             LRT_LIBRCORE_HASHTABLE_KEY_TYPE,           \
             PAYLOAD_TYPE,                              \
             1,                                         \
             kh_int_hash_func,                          \
             kh_int_hash_equal);

#endif
