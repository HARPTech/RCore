#if defined(__AVR__) && !defined(kroundup32)
#define kroundup32(x) \
  (--(x),             \
   (x) |= (x) >> 1,   \
   (x) |= (x) >> 2,   \
   (x) |= (x) >> 4,   \
   (x) |= (x) >> 8,   \
   ++(x))
#endif
