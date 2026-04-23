#include "protocol_fc.h"

#include "app_context.h"
#include "bsp_uart.h"
#include "bsp_timer.h"

#define FC_FRAME_HEAD              0xAAU
#define FC_FRAME_D_ADDR            0xFFU
#define FC_FRAME_ID                0xE0U
#define FC_FRAME_PAYLOAD_LEN       11U
#define FC_FRAME_TOTAL_LEN         (4U + FC_FRAME_PAYLOAD_LEN + 2U)

#define FC_CMD_CID                 0x50U
#define FC_CMD0                    0x00U
#define FC_CMD1_TAKEOFF            0x01U
#define FC_CMD1_LAND               0x02U

static void ProtocolFC_FillChecksum(uint8_t *frame)
{
    uint8_t i;
    uint8_t sum_check = 0U;
    uint8_t add_check = 0U;

    for (i = 0U; i < (FC_FRAME_TOTAL_LEN - 2U); i++)
    {
        sum_check = (uint8_t)(sum_check + frame[i]);
        add_check = (uint8_t)(add_check + sum_check);
    }

    frame[FC_FRAME_TOTAL_LEN - 2U] = sum_check;
    frame[FC_FRAME_TOTAL_LEN - 1U] = add_check;
}

static void ProtocolFC_SendCommand(uint8_t cmd1)
{
    uint8_t frame[FC_FRAME_TOTAL_LEN] = {0U};

    frame[0] = FC_FRAME_HEAD;
    frame[1] = FC_FRAME_D_ADDR;
    frame[2] = FC_FRAME_ID;
    frame[3] = FC_FRAME_PAYLOAD_LEN;
    frame[4] = FC_CMD_CID;
    frame[5] = FC_CMD0;
    frame[6] = cmd1;

    ProtocolFC_FillChecksum(frame);
    (void)BSP_UART_Send(UART_PORT_FC, frame, FC_FRAME_TOTAL_LEN);
}

void ProtocolFC_Init(void)
{
}

void ProtocolFC_ParseBytes(const uint8_t *data, uint16_t len)
{
    if ((data != 0) && (len > 0U))
    {
        g_app.last_fc_rx_ms = BSP_Timer_NowMs();
        g_app.fc_online = 1U;
    }
}

void ProtocolFC_SendTakeoff(float z)
{
    (void)z;
    ProtocolFC_SendCommand(FC_CMD1_TAKEOFF);
}

void ProtocolFC_SendPositionTarget(float x, float y, float z, float yaw)
{
    (void)x;
    (void)y;
    (void)z;
    (void)yaw;
}

void ProtocolFC_SendLand(void)
{
    ProtocolFC_SendCommand(FC_CMD1_LAND);
}
