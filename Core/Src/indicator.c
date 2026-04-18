#include "indicator.h"

#include "bsp_timer.h"
#include "main.h"

static uint8_t s_led_on;
static uint32_t s_last_toggle_ms;

void Indicator_Init(void)
{
    s_led_on = 1U;
    s_last_toggle_ms = 0U;
    Indicator_SetLed(s_led_on);
}

void Indicator_SetLed(uint8_t on)
{
    s_led_on = (on != 0U) ? 1U : 0U;
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, (s_led_on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Indicator_Update(SystemContext_t *ctx)
{
    uint32_t now_ms;
    uint32_t period_ms;

    if (ctx == NULL)
    {
        return;
    }

    now_ms = BSP_Timer_NowMs();
    period_ms = (ctx->state == MS_WAIT_START) ? 1000U : 250U;

    if ((ctx->state == MS_TAKEOFF) || (ctx->state == MS_ABORT) || (ctx->state == MS_ERROR))
    {
        period_ms = 100U;
    }

    if ((ctx->state == MS_WAIT_START) || (ctx->state == MS_TAKEOFF) ||
        (ctx->state == MS_ABORT) || (ctx->state == MS_ERROR))
    {
        if ((s_last_toggle_ms == 0U) || (BSP_Timer_IsElapsed(now_ms, s_last_toggle_ms, period_ms) != 0U))
        {
            s_last_toggle_ms = now_ms;
            Indicator_SetLed((uint8_t)(s_led_on == 0U));
        }
    }
    else
    {
        Indicator_SetLed(1U);
    }
}
