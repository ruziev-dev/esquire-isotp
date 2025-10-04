#ifndef __ISOTP_SERVICE_H__
#define __ISOTP_SERVICE_H__

#include "isotp_user.h"

void IsoTpService_Run(uint8_t NodeId);

int sendIsoTpCmdChannel(uint8_t *payload, uint16_t size);

#endif
