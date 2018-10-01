#include "../include/RCore/internal/avr_kroundup.h"
#include "../include/RCore/defaults.h"

rcomm_handle_t*
rcomm_handle_create_from_config(lrt_rcore_rcomm_config_t config)
{
  return rcomm_create(config.message_default_reserved_memory,
                      config.maximum_buffer_size,
                      config.maximum_stack_size,
                      config.maximum_queue_size,
                      config.message_config);
}

void
rcomm_handle_init_from_config(rcomm_handle_t* handle,
                              lrt_rcore_rcomm_config_t config)
{
  rcomm_init(handle,
             config.message_default_reserved_memory,
             config.maximum_buffer_size,
             config.maximum_stack_size,
             config.maximum_queue_size,
             config.message_config);
}
