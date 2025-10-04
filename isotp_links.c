#include <stdint.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "assert.h"
#include "task_priorities.h"
#include "isotp_links.h"
#include "isotp_link_containers.h"
#include "isotp.h"
#include "mutexes.h"
#include "isotp_tasks.h"
#include "Esqire_ecosystem.h"

void configure_isotp_link(int index, uint32_t receive_arbitration_id, uint32_t reply_arbitration_id, const char *name)
{
    IsoTpLinkContainer *isotp_link_container = &isotp_link_containers[index];
    memset(isotp_link_container, 0, sizeof(IsoTpLinkContainer));
    isotp_link_container->wait_for_isotp_data_sem = xSemaphoreCreateBinary();
    isotp_link_container->recv_buf = calloc(1, ISOTP_BUFSIZE);
    isotp_link_container->send_buf = calloc(1, ISOTP_BUFSIZE);
    isotp_link_container->payload_buf = calloc(1, ISOTP_BUFSIZE);
    assert(isotp_link_container->recv_buf != NULL);
    assert(isotp_link_container->send_buf != NULL);
    assert(isotp_link_container->payload_buf != NULL);
    isotp_init_link(&isotp_link_container->link, receive_arbitration_id, reply_arbitration_id,
                    isotp_link_container->send_buf, ISOTP_BUFSIZE,
                    isotp_link_container->recv_buf, ISOTP_BUFSIZE);
    xTaskCreate(isotp_processing_task, name, configMINIMAL_STACK_SIZE, isotp_link_container, ISOTP_TSK_PRIO, NULL);
}

void configure_isotp_links(uint8_t NodeId)
{
    // acquire lock
    xSemaphoreTake(isotp_mutex, (TickType_t)100);

    // Proto Messages communication
    configure_isotp_link(CMD_LINK_CHANNEL, GET_NODE_CAN_ADDRESS(PROTO_MSG_TRANFER_RX_ID, NodeId), GET_NODE_CAN_ADDRESS(PROTO_MSG_TRANFER_TX_ID, NodeId), "cmd");

    // Big buffers tranfers (for files, configs, firmware updates)
    configure_isotp_link(FILE_TRANSFER_LINK_CHANNEL, GET_NODE_CAN_ADDRESS(FILE_TRANFER_RX_ID, NodeId), GET_NODE_CAN_ADDRESS(FILE_TRANFER_TX_ID, NodeId), "file");

    // configure_isotp_link(1, 0x7E9, 0x7E1, "tcu");

    // free lock
    xSemaphoreGive(isotp_mutex);
}