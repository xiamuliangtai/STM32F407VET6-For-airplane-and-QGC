#ifndef __PROTOCOL_GS_H
#define __PROTOCOL_GS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "protocol.h"

void ProtocolGS_Init(void);
void ProtocolGS_HandleFrame(const ProtoFrame_t *frame);
void ProtocolGS_SendAck(uint8_t seq, uint8_t ack_msg);
void ProtocolGS_SendNack(uint8_t seq, uint8_t nack_msg, uint8_t nack_code);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_GS_H */
