#include "protocol.h"

#include <string.h>

#include "app_context.h"
#include "bsp_uart.h"
#include "protocol_fc.h"
#include "protocol_gs.h"
#include "protocol_openmv.h"
#include "protocol_uwb.h"

static uint16_t read_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
}

static void Protocol_DispatchGsRaw(const UartRxFrame_t *raw)
{
    ProtoFrame_t frame;
    uint16_t offset = 0U;
    uint16_t payload_len;
    uint16_t frame_len;
    ProtocolParseResult_e parse_result;
    uint8_t msg_id;
    uint8_t seq;

    while ((raw != NULL) && ((offset + 8U) <= raw->len))
    {
        if ((raw->data[offset] != PROTO_SOF1) || (raw->data[offset + 1U] != PROTO_SOF2))
        {
            offset++;
            continue;
        }

        msg_id = raw->data[offset + 2U];
        seq = raw->data[offset + 3U];
        payload_len = read_u16_le(&raw->data[offset + 4U]);

        if (payload_len > APP_PROTO_MAX_PAYLOAD_LEN)
        {
            if (msg_id == MSG_UPLOAD_PATH)
            {
                ProtocolGS_SendUploadAck(seq, GS_UPLOAD_ACK_LENGTH_ERROR, 0U);
            }
            g_app.fault_code = FAULT_PROTOCOL;
            break;
        }

        frame_len = (uint16_t)(payload_len + 8U);
        if ((offset + frame_len) > raw->len)
        {
            g_app.fault_code = FAULT_PROTOCOL;
            break;
        }

        parse_result = Protocol_Parse(&raw->data[offset], frame_len, &frame);
        if (parse_result == PROTO_PARSE_OK)
        {
            ProtocolGS_HandleFrame(&frame);
        }
        else
        {
            if (msg_id == MSG_UPLOAD_PATH)
            {
                if (parse_result == PROTO_PARSE_ERR_CRC)
                {
                    ProtocolGS_SendUploadAck(seq, GS_UPLOAD_ACK_CRC_ERROR, 0U);
                }
                else if (parse_result == PROTO_PARSE_ERR_LENGTH)
                {
                    ProtocolGS_SendUploadAck(seq, GS_UPLOAD_ACK_LENGTH_ERROR, 0U);
                }
            }
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

ProtocolParseResult_e Protocol_Parse(const uint8_t *raw, uint16_t len, ProtoFrame_t *out)
{
    uint16_t crc_calc;
    uint16_t crc_recv;
    uint16_t payload_len;

    if ((raw == NULL) || (out == NULL) || (len < 8U))
    {
        return PROTO_PARSE_ERR_LENGTH;
    }

    if ((raw[0] != PROTO_SOF1) || (raw[1] != PROTO_SOF2))
    {
        return PROTO_PARSE_ERR_HEADER;
    }

    payload_len = read_u16_le(&raw[4]);
    if ((payload_len > APP_PROTO_MAX_PAYLOAD_LEN) || (len != (uint16_t)(payload_len + 8U)))
    {
        return PROTO_PARSE_ERR_LENGTH;
    }

    crc_calc = Protocol_Crc16(&raw[2], (uint16_t)(payload_len + 4U));
    crc_recv = read_u16_le(&raw[len - 2U]);
    if (crc_calc != crc_recv)
    {
        return PROTO_PARSE_ERR_CRC;
    }

    out->payload_len = payload_len;
    out->msg_id = raw[2];
    out->seq = raw[3];
    if (payload_len > 0U)
    {
        memcpy(out->payload, &raw[6], payload_len);
    }

    return PROTO_PARSE_OK;
}

uint16_t Protocol_BuildFrame(uint8_t msg_id,
                             uint8_t seq,
                             const uint8_t *payload,
                             uint16_t payload_len,
                             uint8_t *out_buf)
{
    uint16_t crc;

    if ((out_buf == NULL) || (payload_len > APP_PROTO_MAX_PAYLOAD_LEN))
    {
        return 0U;
    }

    out_buf[0] = PROTO_SOF1;
    out_buf[1] = PROTO_SOF2;
    out_buf[2] = msg_id;
    out_buf[3] = seq;
    out_buf[4] = (uint8_t)(payload_len & 0x00FFU);
    out_buf[5] = (uint8_t)((payload_len >> 8U) & 0x00FFU);

    if ((payload_len > 0U) && (payload != NULL))
    {
        memcpy(&out_buf[6], payload, payload_len);
    }

    crc = Protocol_Crc16(&out_buf[2], (uint16_t)(payload_len + 4U));
    out_buf[6U + payload_len] = (uint8_t)(crc & 0x00FFU);
    out_buf[7U + payload_len] = (uint8_t)((crc >> 8U) & 0x00FFU);

    return (uint16_t)(payload_len + 8U);
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
