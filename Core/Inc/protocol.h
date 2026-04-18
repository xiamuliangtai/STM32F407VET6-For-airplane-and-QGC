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
    MSG_HEARTBEAT        = 0x01,
    MSG_ACK              = 0x02,
    MSG_NACK             = 0x03,
    MSG_TASK_START       = 0x10,
    MSG_TASK_STOP        = 0x11,
    MSG_TASK_PAUSE       = 0x12,
    MSG_TASK_RESUME      = 0x13,
    MSG_TASK_CLEAR       = 0x20,
    MSG_TASK_INFO        = 0x21,
    MSG_WAYPOINT_DATA    = 0x22,
    MSG_TASK_COMMIT      = 0x23,
    MSG_STATE_REPORT     = 0x30,
    MSG_DETECTION_REPORT = 0x31,
    MSG_FAULT_REPORT     = 0x32
} MsgId_e;

typedef struct
{
    uint8_t msg_id;
    uint8_t seq;
    uint8_t payload_len;
    uint8_t payload[APP_PROTO_MAX_PAYLOAD_LEN];
} ProtoFrame_t;

uint16_t Protocol_Crc16(const uint8_t *data, uint16_t len);
uint8_t Protocol_Parse(const uint8_t *raw, uint16_t len, ProtoFrame_t *out);
uint16_t Protocol_BuildFrame(uint8_t msg_id,
                             uint8_t seq,
                             const uint8_t *payload,
                             uint8_t payload_len,
                             uint8_t *out_buf);
void Protocol_Dispatch(void);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_H */
