#ifndef __PROTOCOL_GS_H
#define __PROTOCOL_GS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "protocol.h"

typedef enum
{
    GS_UPLOAD_ACK_OK = 0x00,
    GS_UPLOAD_ACK_CRC_ERROR = 0x01,
    GS_UPLOAD_ACK_LENGTH_ERROR = 0x02,
    GS_UPLOAD_ACK_POINT_ERROR = 0x03,
    GS_UPLOAD_ACK_BUFFER_FULL = 0x04
} GsUploadAckResult_e;

void ProtocolGS_Init(void);
void ProtocolGS_HandleFrame(const ProtoFrame_t *frame);
void ProtocolGS_SendUploadAck(uint8_t seq, GsUploadAckResult_e result, uint16_t accepted_count);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_GS_H */
