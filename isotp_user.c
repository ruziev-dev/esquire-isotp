#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "can.h"
#include "isotp.h"
#include "queues.h"
#include "messages.h"

#define ISOTP_CALLBACKS_TAG "isotp_callbacks"

int isotp_user_send_can(const uint32_t arbitration_id, const uint8_t *data, const uint8_t size)
{
    can_message_t frame = {
        .identifier = arbitration_id,
        .data_length_code = size};
    memcpy(frame.data, data, sizeof(frame.data));

    xQueueSend(tx_task_queue, &frame, portMAX_DELAY);

    return ISOTP_RET_OK;
}

uint32_t isotp_user_get_us(void)
{
    return HAL_GetTick() * 1000;
}

#define ISOTP_DEBUG
void isotp_user_debug(const char *message, ...)
{
#ifdef ISOTP_DEBUG
    va_list args;
    va_start(args, message);
    printf("[ISOTP] ");
    printf(message, args);
    va_end(args);
#endif
}