#include "protocol_uwb.h"

#include "app_context.h"
#include "bsp_timer.h"

void ProtocolUWB_Init(void)
{
}

void ProtocolUWB_ParseBytes(const uint8_t *data, uint16_t len)
{
    if ((data != 0) && (len > 0U))
    {
        g_app.last_uwb_rx_ms = BSP_Timer_NowMs();
        g_app.uwb_online = 1U;
    }
}

uint8_t ProtocolUWB_IsDataFresh(uint32_t now_ms)
{
    return ((now_ms - g_app.last_uwb_rx_ms) <= APP_UWB_TIMEOUT_MS) ? 1U : 0U;
}
