#include "protocol.h"

#include <string.h>

#include "app_context.h"
#include "bsp_uart.h"
#include "protocol_fc.h"
#include "protocol_gs.h"
#include "protocol_openmv.h"
#include "protocol_uwb.h"

static void Protocol_DispatchGsRaw(const UartRxFrame_t *raw)
{
    ProtoFrame_t frame;
    uint16_t offset = 0U;
    uint16_t frame_len;

    while ((raw != NULL) && ((offset + 7U) <= raw->len))
    {
        if ((raw->data[offset] != PROTO_SOF1) || (raw->data[offset + 1U] != PROTO_SOF2))
        {
            offset++;
            continue;
        }

        frame_len = (uint16_t)raw->data[offset + 2U] + 7U;
        if ((frame_len < 7U) || ((offset + frame_len) > raw->len))
        {
            g_app.fault_code = FAULT_PROTOCOL;
            break;
        }

        if (Protocol_Parse(&raw->data[offset], frame_len, &frame) != 0U)
        {
            ProtocolGS_HandleFrame(&frame);
        }
        else
        {
            g_app.fault_code = FAULT_PROTOCOL;
        }

        offset = (uint16_t)(offset + frame_len);
    }
}

uint16_t Protocol_Crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i;
    uint8_t j;

    for (i = 0U; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0U; j < 8U; j++)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc = (crc >> 1U) ^ 0xA001U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

uint8_t Protocol_Parse(const uint8_t *raw, uint16_t len, ProtoFrame_t *out)
{
    uint16_t crc_calc;
    uint16_t crc_recv;
    uint16_t payload_len;

    if ((raw == NULL) || (out == NULL) || (len < 7U))
    {
        return 0U;
    }

    if ((raw[0] != PROTO_SOF1) || (raw[1] != PROTO_SOF2))
    {
        return 0U;
    }

    payload_len = raw[2];
    if ((payload_len > APP_PROTO_MAX_PAYLOAD_LEN) || (len != (uint16_t)(payload_len + 7U)))
    {
        return 0U;
    }

    crc_calc = Protocol_Crc16(&raw[2], (uint16_t)(payload_len + 3U));
    crc_recv = (uint16_t)raw[len - 2U] | ((uint16_t)raw[len - 1U] << 8U);
    if (crc_calc != crc_recv)
    {
        return 0U;
    }

    out->payload_len = (uint8_t)payload_len;
    out->msg_id = raw[3];
    out->seq = raw[4];
    if (payload_len > 0U)
    {
        memcpy(out->payload, &raw[5], payload_len);
    }

    return 1U;
}

uint16_t Protocol_BuildFrame(uint8_t msg_id,
                             uint8_t seq,
                             const uint8_t *payload,
                             uint8_t payload_len,
                             uint8_t *out_buf)
{
    uint16_t crc;

    if ((out_buf == NULL) || (payload_len > APP_PROTO_MAX_PAYLOAD_LEN))
    {
        return 0U;
    }

    out_buf[0] = PROTO_SOF1;
    out_buf[1] = PROTO_SOF2;
    out_buf[2] = payload_len;
    out_buf[3] = msg_id;
    out_buf[4] = seq;

    if ((payload_len > 0U) && (payload != NULL))
    {
        memcpy(&out_buf[5], payload, payload_len);
    }

    crc = Protocol_Crc16(&out_buf[2], (uint16_t)(payload_len + 3U));
    out_buf[5U + payload_len] = (uint8_t)(crc & 0x00FFU);
    out_buf[6U + payload_len] = (uint8_t)((crc >> 8U) & 0x00FFU);

    return (uint16_t)(payload_len + 7U);
}

void Protocol_Dispatch(void)
{
    UartRxFrame_t raw;

    if (BSP_UART_FetchFrame(UART_PORT_GS, &raw) != 0U)
    {
        Protocol_DispatchGsRaw(&raw);
    }

    if (BSP_UART_FetchFrame(UART_PORT_FC, &raw) != 0U)
    {
        ProtocolFC_ParseBytes(raw.data, raw.len);
    }

    if (BSP_UART_FetchFrame(UART_PORT_UWB, &raw) != 0U)
    {
        ProtocolUWB_ParseBytes(raw.data, raw.len);
    }

    if (BSP_UART_FetchFrame(UART_PORT_OPENMV, &raw) != 0U)
    {
        ProtocolOpenMV_ParseBytes(raw.data, raw.len);
    }
}
