#ifndef __ISOTP_MUTEXES_H__
#define __ISOTP_MUTEXES_H__

#include "semphr.h"

extern SemaphoreHandle_t done_sem;
extern SemaphoreHandle_t isotp_send_queue_sem;
extern SemaphoreHandle_t isotp_mutex;

#endif