#ifndef LRT_LIBRCORE_SEQUENCE_STACK_H
#define LRT_LIBRCORE_SEQUENCE_STACK_H

#ifdef __cplusplus
extern "C"
{
#endif

#define LRT_RCORE_SEQUENCE_STACK(sPREFIX, iSTACK_WIDTH, iSTACK_DEPTH) \
  typedef struct sPREFIX##_sequence_stack_t                           \
  {                                                                   \
    struct entry_t                                                    \
    {                                                                 \
      uint8_t liteCommType;                                           \
      uint16_t liteCommProperty;                                      \
      sPREFIX##_block_t blocks[iSTACK_DEPTH];                         \
    } entries[iSTACK_WIDTH];                                          \
  } sPREFIX##_sequence_stack_t;

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
