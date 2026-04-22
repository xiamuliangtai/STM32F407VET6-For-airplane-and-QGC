#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_config.h"

#define PROTO_SOF1    0xAAU
#define PROTO_SOF2    0x55U

typedef enum
{
    MSG_UPLOAD_PATH     = 0x01,
    MSG_ANIMAL_REPORT   = 0x02,
    MSG_ACK_UPLOAD_PATH = 0x81
} MsgId_e;

typedef enum
{
    PROTO_PARSE_OK = 0,
    PROTO_PARSE_ERR_HEADER,
    PROTO_PARSE_ERR_LENGTH,
    PROTO_PARSE_ERR_CRC
} ProtocolParseResult_e;

typedef struct
{
    uint8_t msg_id;
    uint8_t seq;
    uint16_t payload_len;
    uint8_t payload[APP_PROTO_MAX_PAYLOAD_LEN];
} ProtoFrame_t;

uint16_t Protocol_Crc16(const uint8_t *data, uint16_t len);
ProtocolParseResult_e Protocol_Parse(const uint8_t *raw, uint16_t len, ProtoFrame_t *out);
uint16_t Protocol_BuildFrame(uint8_t msg_id,
                             uint8_t seq,
                             const uint8_t *payload,
                             uint16_t payload_len,
                             uint8_t *out_buf);
void Protocol_Dispatch(void);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_H */
