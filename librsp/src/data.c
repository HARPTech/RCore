#include "../include/RCore/librsp/data.h"

size_t
lrt_size_t_min(size_t a, size_t b);
size_t
rcomm_message_get_data_offset(const lrt_rbp_message_t* message);
size_t
rcomm_message_get_data_size(const lrt_rbp_message_t* message);
size_t
rcomm_message_insert_data(lrt_rbp_message_t* message,
                          const uint8_t* data,
                          size_t length,
                          size_t offset);
size_t
rcomm_message_read_data(const lrt_rbp_message_t* message,
                        uint8_t* target,
                        size_t target_length,
                        size_t offset);
