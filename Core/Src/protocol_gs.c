#include "protocol_gs.h"

#include <string.h>

#include "app_context.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "mission.h"

#define GS_TASK_INFO_LEN          4U
#define GS_WAYPOINT_DATA_LEN      17U

#define GS_ERR_BAD_PARAM          1U
#define GS_ERR_BAD_STATE          2U
#define GS_ERR_TASK_INVALID       3U

static uint16_t read_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
}

static float read_float_le(const uint8_t *data)
{
    float value;
    memcpy(&value, data, sizeof(value));
    return value;
}

void ProtocolGS_Init(void)
{
}

void ProtocolGS_SendAck(uint8_t seq, uint8_t ack_msg)
{
    uint8_t payload[1];
    uint8_t frame[APP_PROTO_MAX_FRAME_LEN];
    uint16_t len;

    payload[0] = ack_msg;
    len = Protocol_BuildFrame(MSG_ACK, seq, payload, sizeof(payload), frame);
    if (len > 0U)
    {
        (void)BSP_UART_Send(UART_PORT_GS, frame, len);
    }
}

void ProtocolGS_SendNack(uint8_t seq, uint8_t nack_msg, uint8_t nack_code)
{
    uint8_t payload[2];
    uint8_t frame[APP_PROTO_MAX_FRAME_LEN];
    uint16_t len;

    payload[0] = nack_msg;
    payload[1] = nack_code;
    len = Protocol_BuildFrame(MSG_NACK, seq, payload, sizeof(payload), frame);
    if (len > 0U)
    {
        (void)BSP_UART_Send(UART_PORT_GS, frame, len);
    }
}

void ProtocolGS_HandleFrame(const ProtoFrame_t *frame)
{
    WayPoint_t wp;
    uint16_t index;

    if (frame == NULL)
    {
        return;
    }

    g_app.last_gs_rx_ms = BSP_Timer_NowMs();
    g_app.gs_online = 1U;

    switch (frame->msg_id)
    {
    case MSG_HEARTBEAT:
        ProtocolGS_SendAck(frame->seq, frame->msg_id);
        break;

    case MSG_TASK_CLEAR:
        Mission_Clear(&g_app.mission);
        g_app.start_requested = 0U;
        g_app.fault_code = FAULT_NONE;
        ProtocolGS_SendAck(frame->seq, frame->msg_id);
        break;

    case MSG_TASK_INFO:
        if (frame->payload_len != GS_TASK_INFO_LEN)
        {
            ProtocolGS_SendNack(frame->seq, frame->msg_id, GS_ERR_BAD_PARAM);
            break;
        }

        if (Mission_SetInfo(&g_app.mission,
                            read_u16_le(&frame->payload[0]),
                            read_u16_le(&frame->payload[2])) == 0U)
        {
            ProtocolGS_SendNack(frame->seq, frame->msg_id, GS_ERR_TASK_INVALID);
            break;
        }

        ProtocolGS_SendAck(frame->seq, frame->msg_id);
        break;

    case MSG_WAYPOINT_DATA:
        if (frame->payload_len != GS_WAYPOINT_DATA_LEN)
        {
            ProtocolGS_SendNack(frame->seq, frame->msg_id, GS_ERR_BAD_PARAM);
            break;
        }

        index = read_u16_le(&frame->payload[0]);
        wp.x = read_float_le(&frame->payload[2]);
        wp.y = read_float_le(&frame->payload[6]);
        wp.z = read_float_le(&frame->payload[10]);
        wp.type = frame->payload[14];
        wp.hold_ms = read_u16_le(&frame->payload[15]);

        if (Mission_SetWaypoint(&g_app.mission, index, &wp) == 0U)
        {
            ProtocolGS_SendNack(frame->seq, frame->msg_id, GS_ERR_TASK_INVALID);
            break;
        }

        ProtocolGS_SendAck(frame->seq, frame->msg_id);
        break;

    case MSG_TASK_COMMIT:
        if (Mission_Commit(&g_app.mission) == 0U)
        {
            g_app.fault_code = FAULT_TASK_INVALID;
            ProtocolGS_SendNack(frame->seq, frame->msg_id, GS_ERR_TASK_INVALID);
            break;
        }

        g_app.fault_code = FAULT_NONE;
        ProtocolGS_SendAck(frame->seq, frame->msg_id);
        break;

    case MSG_TASK_START:
        if (Mission_IsReady(&g_app.mission) == 0U)
        {
            ProtocolGS_SendNack(frame->seq, frame->msg_id, GS_ERR_BAD_STATE);
            break;
        }

        g_app.start_requested = 1U;
        ProtocolGS_SendAck(frame->seq, frame->msg_id);
        break;

    default:
        ProtocolGS_SendNack(frame->seq, frame->msg_id, GS_ERR_BAD_PARAM);
        break;
    }
}
