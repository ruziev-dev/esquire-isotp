#include "FreeRTOS.h"
#include "isotp.h"
#include "mutexes.h"
#include "queues.h"
#include "rx_tx_tasks.h"
#include "isotp_link_containers.h"
#include "can.h"
#include "messages.h"
#include "stm32f4xx_hal_can.h"

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef msgHeader;

    can_message_t frame;

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &msgHeader, frame.data);

    if (msgHeader.IDE == CAN_ID_EXT)
    {
        frame.identifier = msgHeader.ExtId;
    }
    else
    {
        frame.identifier = msgHeader.StdId;
    }
    frame.data_length_code = msgHeader.DLC;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(rx_task_queue, &frame, &xHigherPriorityTaskWoken);
}

void can_receive_task()
{
    can_message_t rx_message;
    while (1)
    {
        xQueueReceive(rx_task_queue, &rx_message, portMAX_DELAY);

        int isotp_link_container_index = find_isotp_link_container_index_by_receive_arbitration_id(rx_message.identifier);

        if (isotp_link_container_index != -1)
        {
            IsoTpLinkContainer *isotp_link_container = &isotp_link_containers[isotp_link_container_index];
            xSemaphoreTake(isotp_mutex, (TickType_t)100);
            isotp_on_can_message(&isotp_link_container->link, rx_message.data, rx_message.data_length_code);

            xSemaphoreGive(isotp_mutex);
            xSemaphoreGive(isotp_link_container->wait_for_isotp_data_sem);
        }
    }
}

void can_transmit_task()
{
    can_message_t tx_message;
    while (1)
    {

        xQueueReceive(tx_task_queue, &tx_message, portMAX_DELAY);

        CAN_TxHeaderTypeDef txHeader;
        txHeader.StdId = tx_message.identifier;
        txHeader.DLC = tx_message.data_length_code;
        txHeader.TransmitGlobalTime = DISABLE;
        txHeader.RTR = CAN_RTR_DATA;
        txHeader.IDE = CAN_ID_STD;

        uint32_t mailBoxNum = 0;

        HAL_StatusTypeDef ret = HAL_CAN_AddTxMessage(&hcan1, &txHeader, tx_message.data, &mailBoxNum);
    }
    vTaskDelete(NULL);
}