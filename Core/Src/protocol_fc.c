#include "protocol_fc.h"

#include "app_context.h"
#include "bsp_timer.h"

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
}
