#ifndef LRT_LIBRSP_BYTE_SWAP_H
#define LRT_LIBRSP_BYTE_SWAP_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NO_BYTESWAP_H
#include <byteswap.h>
#else
#include <stdint.h>
static inline uint16_t
__bswap_16(uint16_t x)
{
  union
  {
    uint16_t x;
    struct
    {
      uint8_t a;
      uint8_t b;
    } s;
  } in, out;
  in.x = x;
  out.s.a = in.s.b;
  out.s.b = in.s.a;
  return out.x;
}

static inline uint32_t
__bswap_32(uint32_t x)
{
  union
  {
    uint32_t x;
    struct
    {
      uint8_t a;
      uint8_t b;
      uint8_t c;
      uint8_t d;
    } s;
  } in, out;
  in.x = x;
  out.s.a = in.s.d;
  out.s.b = in.s.c;
  out.s.c = in.s.b;
  out.s.d = in.s.a;
  return out.x;
}
#endif

#if BIG_ENDIAN == 1
#define LRT_LIBRSP_BYTESWAP16_IF_NEEDED(VALUE) VALUE = __bswap_16(VALUE)
#define LRT_LIBRSP_BYTESWAP32_IF_NEEDED(VALUE) VALUE = __bswap_32(VALUE)
#else
#define LRT_LIBRSP_BYTESWAP16_IF_NEEDED(VALUE)
#define LRT_LIBRSP_BYTESWAP32_IF_NEEDED(VALUE)
#endif

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
