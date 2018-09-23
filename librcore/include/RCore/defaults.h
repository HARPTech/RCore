#ifndef LRT_LIBRCORE_DEFAULTS_H
#define LRT_LIBRCORE_DEFAULTS_H

#include "rcomm.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct lrt_rcore_rcomm_config_t
  {
    size_t message_default_reserved_memory;
    size_t maximum_buffer_size;
    size_t maximum_stack_size;
    size_t maximum_queue_size;
    lrt_rbp_message_config_type message_config;
  } lrt_rcore_rcomm_config_t;

  const lrt_rcore_rcomm_config_t lrt_rcore_rcomm_universal_defaults =
    { 14, 14, 8, 8, LRT_RBP_MESSAGE_CONFIG_ENABLE_CRC8 };

  rcomm_handle_t* rcomm_handle_create_from_config(
    lrt_rcore_rcomm_config_t config);

  void rcomm_handle_init_from_config(rcomm_handle_t* handle,
                                     lrt_rcore_rcomm_config_t config);

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
