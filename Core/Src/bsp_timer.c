#include "bsp_timer.h"

#include "main.h"

void BSP_Timer_Init(void)
{
}

uint32_t BSP_Timer_NowMs(void)
{
    return HAL_GetTick();
}

uint8_t BSP_Timer_IsElapsed(uint32_t now, uint32_t start, uint32_t period_ms)
{
    return ((now - start) >= period_ms) ? 1U : 0U;
}
