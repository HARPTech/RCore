#include "../include/RCore/librsp/litecomm.h"

uint8_t
rcomm_message_get_litecomm_type(const lrt_rbp_message_t* message);
void
rcomm_message_set_litecomm_type(lrt_rbp_message_t* message, uint8_t type);
uint16_t
rcomm_message_get_litecomm_property(const lrt_rbp_message_t* message);
void
rcomm_message_set_litecomm_property(lrt_rbp_message_t* message,
                                    uint16_t property);
uint16_t
rcomm_message_get_sequence_number(const lrt_rbp_message_t* message);
void
rcomm_message_set_sequence_number(lrt_rbp_message_t* message, uint16_t seq);
