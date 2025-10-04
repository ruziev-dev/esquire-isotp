#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "Esqire_ecosystem.h"
#include "isotp_link_containers.h"
#include "task_priorities.h"
#include "isotp.h"
#include "mutexes.h"
#include "messages.h"
#include "queues.h"
#include "main.h"

void isotp_processing_task(void *arg)
{
    IsoTpLinkContainer *isotp_link_container = (IsoTpLinkContainer *)arg;
    IsoTpLink *link_ptr = &isotp_link_container->link;
    uint8_t *payload_buf = isotp_link_container->payload_buf;
    while (1)
    {

        if (link_ptr->send_status != ISOTP_SEND_STATUS_INPROGRESS &&
            link_ptr->receive_status != ISOTP_RECEIVE_STATUS_INPROGRESS)
        {
            // Link is idle, wait for new data before pumping loop.
            xSemaphoreTake(isotp_link_container->wait_for_isotp_data_sem, portMAX_DELAY);
        }
        // poll
        xSemaphoreTake(isotp_mutex, (TickType_t)100);
        isotp_poll(link_ptr);
        xSemaphoreGive(isotp_mutex);
        // receive
        xSemaphoreTake(isotp_mutex, (TickType_t)100);
        uint16_t out_size;
        int ret = isotp_receive(link_ptr, payload_buf, ISOTP_BUFSIZE, &out_size);

        xSemaphoreGive(isotp_mutex);

        if (ISOTP_RET_OK == ret)
        {
            isotp_buffer_t payload = {.buffer = payload_buf, .length = out_size};

            // DBG("task: %s receive_arbitration_id: 0x%02lX, send_arbitration_id: 0x%02lX", pcTaskGetName(NULL), link_ptr->receive_arbitration_id, link_ptr->send_arbitration_id);

            if (isotp_link_container == &isotp_link_containers[CMD_LINK_CHANNEL])
            {
                // DBG("task: %s Received ISO-TP message by PROTO_MSG_TRANFER_RX_ID with length: %u", pcTaskGetName(NULL), out_size);
                xQueueSend(handle_ecosystem_cmd_buffer, &payload, 1000);
            }
            else if (isotp_link_container == &isotp_link_containers[FILE_TRANSFER_LINK_CHANNEL])
            {
                // DBG("task: %s Received ISO-TP message by FILE_TRANSFER_LINK_CHANNEL with length: %u", pcTaskGetName(NULL), out_size);
                xQueueSend(handle_ecosystem_file_transfer_buffer, &payload, 10);
            }
            else
            {
                DBG("task: %s INCORREC LINKING", pcTaskGetName(NULL));
            }
        }
        vTaskDelay(0);
    }
    vTaskDelete(NULL);
}

void isotp_send_queue_task(void *arg)
{
    xSemaphoreTake(isotp_send_queue_sem, portMAX_DELAY);
    while (1)
    {
        send_message_t msg;
        xQueueReceive(isotp_send_message_queue, &msg, portMAX_DELAY);
        xSemaphoreTake(isotp_mutex, (TickType_t)100);
        DBG("isotp_send_queue_task: sending message with %d size (rx id: %08x / tx id: %08x)", msg.msg_length, msg.rx_id, msg.tx_id);
        // flipped
        int isotp_link_container_index = find_isotp_link_container_index_by_receive_arbitration_id(msg.tx_id);

        if (isotp_link_container_index == -1)
            continue;

        IsoTpLinkContainer *isotp_link_container = &isotp_link_containers[isotp_link_container_index];
        isotp_send(&isotp_link_container->link, msg.buffer, msg.msg_length);
        // cleanup
        if (msg.reuse_buffer == false)
        {
            free(msg.buffer);
        }
        else
        {
            DBG("isotp_send_queue_task: reusing buffer");
        }
        xSemaphoreGive(isotp_mutex);
        xSemaphoreGive(isotp_link_container->wait_for_isotp_data_sem);
    }
    vTaskDelete(NULL);
}