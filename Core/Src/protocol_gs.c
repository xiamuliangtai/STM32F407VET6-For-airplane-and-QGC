#include "protocol_gs.h"

#include "app_context.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "mission.h"

#define GS_POINT_WIRE_SIZE        6U

static uint8_t s_last_animal_code = 0xFFU;
static uint8_t s_last_col = 0U;
static uint8_t s_last_row = 0U;
static uint8_t s_animal_report_seq = 0U;
static uint32_t s_last_animal_report_ms = 0U;

static uint16_t read_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
}

static void record_upload_status(uint8_t seq, GsUploadAckResult_e result)
{
    uint32_t now_ms;

    now_ms = BSP_Timer_NowMs();
    g_app.last_gs_rx_ms = now_ms;
    g_app.gs_last_rx_event_ms = now_ms;
    g_app.gs_online = 1U;
    g_app.gs_last_rx_seq = seq;
    g_app.gs_last_rx_code = (uint8_t)result;
    g_app.gs_rx_status = (result == GS_UPLOAD_ACK_OK) ? GS_RX_STATUS_OK : GS_RX_STATUS_ERR;
}

void ProtocolGS_Init(void)
{
    s_last_animal_code = 0xFFU;
    s_last_col = 0U;
    s_last_row = 0U;
    s_animal_report_seq = 0U;
    s_last_animal_report_ms = 0U;
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

void ProtocolGS_SendAnimalReport(uint8_t animal_code, uint8_t col, uint8_t row)
{
    uint8_t payload[3];
    uint8_t frame[APP_PROTO_MAX_FRAME_LEN];
    uint16_t len;
    uint32_t now_ms;

    if (animal_code > (uint8_t)GS_ANIMAL_PEACOCK)
    {
        return;
    }

    if ((col < 1U) || (col > 9U) || (row < 1U) || (row > 7U))
    {
        return;
    }

    now_ms = BSP_Timer_NowMs();
    if ((animal_code == s_last_animal_code) &&
        (col == s_last_col) &&
        (row == s_last_row) &&
        (BSP_Timer_IsElapsed(now_ms, s_last_animal_report_ms, APP_ANIMAL_REPORT_MIN_INTERVAL_MS) == 0U))
    {
        return;
    }

    payload[0] = animal_code;
    payload[1] = col;
    payload[2] = row;

    len = Protocol_BuildFrame(MSG_ANIMAL_REPORT, s_animal_report_seq, payload, sizeof(payload), frame);
    if ((len > 0U) && (BSP_UART_Send(UART_PORT_GS, frame, len) == HAL_OK))
    {
        s_last_animal_code = animal_code;
        s_last_col = col;
        s_last_row = row;
        s_last_animal_report_ms = now_ms;
        s_animal_report_seq++;
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
