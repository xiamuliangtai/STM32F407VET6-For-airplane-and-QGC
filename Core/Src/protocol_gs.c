#include "protocol_gs.h"

#include "app_context.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "mission.h"

#define GS_POINT_WIRE_SIZE        6U

static uint16_t read_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
}

static void record_upload_status(uint8_t seq, GsUploadAckResult_e result)
{
    g_app.last_gs_rx_ms = BSP_Timer_NowMs();
    g_app.gs_online = 1U;
    g_app.gs_last_rx_seq = seq;
    g_app.gs_last_rx_code = (uint8_t)result;
    g_app.gs_rx_status = (result == GS_UPLOAD_ACK_OK) ? GS_RX_STATUS_OK : GS_RX_STATUS_ERR;
}

void ProtocolGS_Init(void)
{
}

void ProtocolGS_SendUploadAck(uint8_t seq, GsUploadAckResult_e result, uint16_t accepted_count)
{
    uint8_t payload[4];
    uint8_t frame[APP_PROTO_MAX_FRAME_LEN];
    uint16_t len;

    record_upload_status(seq, result);

    payload[0] = (uint8_t)result;
    payload[1] = 0U;
    payload[2] = (uint8_t)(accepted_count & 0x00FFU);
    payload[3] = (uint8_t)((accepted_count >> 8U) & 0x00FFU);
    len = Protocol_BuildFrame(MSG_ACK_UPLOAD_PATH, seq, payload, sizeof(payload), frame);
    if (len > 0U)
    {
        (void)BSP_UART_Send(UART_PORT_GS, frame, len);
    }
}

void ProtocolGS_HandleFrame(const ProtoFrame_t *frame)
{
    GsPoint_t points[APP_MAX_WAYPOINTS];
    uint16_t expected_len;
    uint16_t payload_offset;
    uint8_t point_count;
    uint8_t idx;

    if (frame == NULL)
    {
        return;
    }

    if (frame->msg_id != MSG_UPLOAD_PATH)
    {
        return;
    }

    if (frame->payload_len < 1U)
    {
        g_app.fault_code = FAULT_TASK_INVALID;
        ProtocolGS_SendUploadAck(frame->seq, GS_UPLOAD_ACK_LENGTH_ERROR, 0U);
        return;
    }

    point_count = frame->payload[0];
    if (point_count == 0U)
    {
        g_app.fault_code = FAULT_TASK_INVALID;
        ProtocolGS_SendUploadAck(frame->seq, GS_UPLOAD_ACK_POINT_ERROR, 0U);
        return;
    }

    if (point_count > APP_MAX_WAYPOINTS)
    {
        g_app.fault_code = FAULT_TASK_INVALID;
        ProtocolGS_SendUploadAck(frame->seq, GS_UPLOAD_ACK_BUFFER_FULL, 0U);
        return;
    }

    expected_len = (uint16_t)(1U + ((uint16_t)point_count * GS_POINT_WIRE_SIZE));
    if (frame->payload_len != expected_len)
    {
        g_app.fault_code = FAULT_TASK_INVALID;
        ProtocolGS_SendUploadAck(frame->seq, GS_UPLOAD_ACK_LENGTH_ERROR, 0U);
        return;
    }

    for (idx = 0U; idx < point_count; idx++)
    {
        payload_offset = (uint16_t)(1U + ((uint16_t)idx * GS_POINT_WIRE_SIZE));
        points[idx].col = frame->payload[payload_offset];
        points[idx].row = frame->payload[payload_offset + 1U];
        points[idx].type = frame->payload[payload_offset + 2U];
        points[idx].hold_ms = read_u16_le(&frame->payload[payload_offset + 3U]);
        points[idx].reserved = frame->payload[payload_offset + 5U];

        if ((points[idx].col < 1U) || (points[idx].col > 9U) ||
            (points[idx].row < 1U) || (points[idx].row > 7U))
        {
            g_app.fault_code = FAULT_TASK_INVALID;
            ProtocolGS_SendUploadAck(frame->seq, GS_UPLOAD_ACK_POINT_ERROR, idx);
            return;
        }
    }

    if (Mission_LoadPath(&g_app.mission, frame->seq, point_count, points) == 0U)
    {
        g_app.fault_code = FAULT_TASK_INVALID;
        ProtocolGS_SendUploadAck(frame->seq, GS_UPLOAD_ACK_BUFFER_FULL, 0U);
        return;
    }

    g_app.start_requested = 0U;
    g_app.fault_code = FAULT_NONE;
    ProtocolGS_SendUploadAck(frame->seq, GS_UPLOAD_ACK_OK, point_count);
}
