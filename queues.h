#ifndef __ISOTP_QUEUES_H__
#define __ISOTP_QUEUES_H__

#include "queue.h"

extern QueueHandle_t tx_task_queue;
extern QueueHandle_t rx_task_queue;
extern QueueHandle_t isotp_send_message_queue;

#endif