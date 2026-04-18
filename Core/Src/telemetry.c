#include "telemetry.h"

#include "bsp_timer.h"
#include "bsp_uart.h"
#include "protocol.h"

static uint8_t s_seq;
static uint32_t s_last_heartbeat_ms;
static uint32_t s_last_state_ms;

static void write_u16_le(uint8_t *buf, uint16_t value)
{
    buf[0] = (uint8_t)(value & 0x00FFU);
    buf[1] = (uint8_t)((value >> 8U) & 0x00FFU);
}

void Telemetry_Init(void)
{
    s_seq = 0U;
    s_last_heartbeat_ms = 0U;
    s_last_state_ms = 0U;
}

void Telemetry_ReportHeartbeat(SystemContext_t *ctx)
{
    uint8_t payload[2];
    uint8_t frame[APP_PROTO_MAX_FRAME_LEN];
    uint16_t len;

    if (ctx == NULL)
    {
        return;
    }

    payload[0] = (uint8_t)ctx->state;
    payload[1] = ctx->gs_online;
    len = Protocol_BuildFrame(MSG_HEARTBEAT, s_seq++, payload, sizeof(payload), frame);
    if (len > 0U)
    {
        (void)BSP_UART_Send(UART_PORT_GS, frame, len);
    }
}

void Telemetry_ReportState(SystemContext_t *ctx)
{
    uint8_t payload[8];
    uint8_t frame[APP_PROTO_MAX_FRAME_LEN];
    uint16_t len;

    if (ctx == NULL)
    {
        return;
    }

    payload[0] = (uint8_t)ctx->state;
    write_u16_le(&payload[1], ctx->mission.task_id);
    write_u16_le(&payload[3], ctx->mission.current_index);
    write_u16_le(&payload[5], ctx->mission.total_points);
    payload[7] = (uint8_t)ctx->fault_code;

    len = Protocol_BuildFrame(MSG_STATE_REPORT, s_seq++, payload, sizeof(payload), frame);
    if (len > 0U)
    {
        (void)BSP_UART_Send(UART_PORT_GS, frame, len);
    }
}

void Telemetry_ReportFault(SystemContext_t *ctx)
{
    uint8_t payload[1];
    uint8_t frame[APP_PROTO_MAX_FRAME_LEN];
    uint16_t len;

    if ((ctx == NULL) || (ctx->fault_code == FAULT_NONE))
    {
        return;
    }

    payload[0] = (uint8_t)ctx->fault_code;
    len = Protocol_BuildFrame(MSG_FAULT_REPORT, s_seq++, payload, sizeof(payload), frame);
    if (len > 0U)
    {
        (void)BSP_UART_Send(UART_PORT_GS, frame, len);
    }
}

void Telemetry_Task(SystemContext_t *ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    if ((s_last_heartbeat_ms == 0U) ||
        (BSP_Timer_IsElapsed(ctx->now_ms, s_last_heartbeat_ms, APP_TELEMETRY_HEARTBEAT_MS) != 0U))
    {
        s_last_heartbeat_ms = ctx->now_ms;
        Telemetry_ReportHeartbeat(ctx);
    }

    if ((s_last_state_ms == 0U) ||
        (BSP_Timer_IsElapsed(ctx->now_ms, s_last_state_ms, APP_TELEMETRY_STATE_MS) != 0U))
    {
        s_last_state_ms = ctx->now_ms;
        Telemetry_ReportState(ctx);
    }

    Telemetry_ReportFault(ctx);
}
