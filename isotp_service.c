
#include "isotp_service.h"

#include "FreeRTOS.h"
#include "rx_tx_tasks.h"
#include "queues.h"
#include "mutexes.h"
#include "isotp_links.h"
#include "isotp_tasks.h"
#include "task_priorities.h"
#include "messages.h"
#include "isotp.h"
#include "isotp_link_containers.h"

#include "can.h"

QueueHandle_t tx_task_queue;
QueueHandle_t rx_task_queue;
QueueHandle_t isotp_send_message_queue;
SemaphoreHandle_t done_sem;
SemaphoreHandle_t isotp_send_queue_sem;
SemaphoreHandle_t isotp_mutex;

void IsoTpService_Run(uint8_t NodeId)
{

    tx_task_queue = xQueueCreate(128, sizeof(can_message_t));
    rx_task_queue = xQueueCreate(128, sizeof(can_message_t));
    isotp_send_message_queue = xQueueCreate(128, sizeof(send_message_t));
    done_sem = xSemaphoreCreateBinary();
    isotp_send_queue_sem = xSemaphoreCreateBinary();
    isotp_mutex = xSemaphoreCreateMutex();

    CAN_FilterTypeDef canFilterConfig;
    canFilterConfig.FilterBank = 0;
    canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilterConfig.FilterIdHigh = 0x0000;
    canFilterConfig.FilterIdLow = 0x0000;
    canFilterConfig.FilterMaskIdHigh = 0x0000;
    canFilterConfig.FilterMaskIdLow = 0x0000;
    canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    canFilterConfig.FilterActivation = ENABLE;
    canFilterConfig.SlaveStartFilterBank = 14;

    HAL_CAN_ConfigFilter(&hcan1, &canFilterConfig);
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY);

    // ISO-TP handler + tasks
    configure_isotp_links(NodeId);
    xSemaphoreGive(isotp_send_queue_sem);

    // Tasks :
    // "TWAI_rx" polls the receive queue (blocking) and once a message exists, forwards it into the ISO-TP library.
    // "TWAI_tx" blocks on a send queue which is populated by the callback from the ISO-TP library
    // "ISOTP_process" pumps the ISOTP library's "poll" method, which will call the send queue callback if a message needs to be sent.
    // ISOTP_process also polls the ISOTP library's non-blocking receive method, which will produce a message if one is ready.
    // "MAIN_process_send_queue" processes queued messages from the BLE stack. These messages are dynamically allocated when they are queued and freed in this task.
    xTaskCreate(can_receive_task, "CAN_rx", configMINIMAL_STACK_SIZE, NULL, RX_TASK_PRIO, NULL);
    xTaskCreate(can_transmit_task, "CAN_tx", configMINIMAL_STACK_SIZE, NULL, TX_TASK_PRIO, NULL);
    xTaskCreate(isotp_send_queue_task, "isotp", configMINIMAL_STACK_SIZE, NULL, MAIN_TSK_PRIO, NULL);
}

int sendIsoTpCmdChannel(uint8_t *payload, uint16_t size)
{

    return isotp_send(&isotp_link_containers[CMD_LINK_CHANNEL].link, payload, size);
}
