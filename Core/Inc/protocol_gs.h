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

typedef enum
{
    GS_ANIMAL_ELEPHANT = 0x00,
    GS_ANIMAL_TIGER = 0x01,
    GS_ANIMAL_WOLF = 0x02,
    GS_ANIMAL_MONKEY = 0x03,
    GS_ANIMAL_PEACOCK = 0x04
} GsAnimalCode_e;

void ProtocolGS_Init(void);
void ProtocolGS_HandleFrame(const ProtoFrame_t *frame);
void ProtocolGS_SendUploadAck(uint8_t seq, GsUploadAckResult_e result, uint16_t accepted_count);
void ProtocolGS_SendAnimalReport(uint8_t animal_code, uint8_t col, uint8_t row, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_GS_H */
