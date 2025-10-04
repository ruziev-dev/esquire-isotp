#ifndef __ISOTP_QUEUES_H__
#define __ISOTP_QUEUES_H__

#include "queue.h"

extern QueueHandle_t tx_task_queue;
extern QueueHandle_t rx_task_queue;
extern QueueHandle_t isotp_send_message_queue;

typedef struct
{
    uint8_t *buffer;
    size_t length;
} isotp_buffer_t;

extern QueueHandle_t handle_ecosystem_cmd_buffer;
extern QueueHandle_t handle_ecosystem_file_transfer_buffer;


#endif