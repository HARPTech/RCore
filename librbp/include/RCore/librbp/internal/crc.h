#ifndef LRT_LIBRBP_INTERNAL_CRC_H
#define LRT_LIBRBP_INTERNAL_CRC_H

#include "../../../../../librcore/include/RCore/events.h"
#include "../message.h"

#ifdef __cplusplus
extern "C"
{
#endif

  lrt_rcore_event_t lrt_rbp_validate_crc(const lrt_rbp_message_t* message);
  lrt_rcore_event_t lrt_rbp_set_crc(lrt_rbp_message_t* message);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
