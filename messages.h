#ifndef __MESSAGES_H__
#define __MESSAGES_H__
#include <stdbool.h>

// Simple struct for a dynamically sized send message

typedef struct send_message
{
    int32_t msg_length;
    uint8_t *buffer;
    uint32_t rx_id;
    uint32_t tx_id;
    bool reuse_buffer;
} send_message_t;

typedef struct can_message
{
    uint32_t identifier;
    int32_t data_length_code;
    uint8_t data[8];

} can_message_t;

#endif