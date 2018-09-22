#include "../include/RCore/librsp/flags.h"

size_t
lrt_size_t_min(size_t a, size_t b);

uint8_t
rcomm_get_packetTypeBits(const lrt_rbp_message_t* message);

bool
rcomm_message_has_flag(const lrt_rbp_message_t* message,
                       enum lrt_rsp_flag flag);

void
rcomm_message_set_flag(lrt_rbp_message_t* message,
                       enum lrt_rsp_flag flag,
                       bool state);
