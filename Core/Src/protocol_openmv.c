#include "protocol_openmv.h"

#include "app_context.h"
#include "bsp_timer.h"

void ProtocolOpenMV_Init(void)
{
}

void ProtocolOpenMV_ParseBytes(const uint8_t *data, uint16_t len)
{
    if ((data != 0) && (len > 0U))
    {
        g_app.last_vision_rx_ms = BSP_Timer_NowMs();
        g_app.vision_online = 1U;
    }
}

uint8_t ProtocolOpenMV_IsDataFresh(uint32_t now_ms)
{
    return ((now_ms - g_app.last_vision_rx_ms) <= APP_VISION_TIMEOUT_MS) ? 1U : 0U;
}
