#include "safety.h"

#include "app_config.h"
#include "bsp_timer.h"
#include "mission_fsm.h"

void Safety_Init(void)
{
}

void Safety_TriggerAbort(SystemContext_t *ctx, FaultCode_e fault)
{
    if (ctx == NULL)
    {
        return;
    }

    ctx->mission_abort = 1U;
    ctx->fault_code = fault;
    MissionFSM_SetState(ctx, MS_ABORT);
}

uint8_t Safety_IsMissionAllowed(const SystemContext_t *ctx)
{
    if (ctx == NULL)
    {
        return 0U;
    }

    return ((ctx->emergency_stop == 0U) && (ctx->mission_abort == 0U)) ? 1U : 0U;
}

void Safety_Check(SystemContext_t *ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    ctx->now_ms = BSP_Timer_NowMs();

    if (ctx->last_gs_rx_ms == 0U)
    {
        ctx->gs_online = 0U;
    }
    else if (BSP_Timer_IsElapsed(ctx->now_ms, ctx->last_gs_rx_ms, APP_GS_TIMEOUT_MS) != 0U)
    {
        ctx->gs_online = 0U;
    }
    else
    {
        ctx->gs_online = 1U;
    }

    if (ctx->emergency_stop != 0U)
    {
        Safety_TriggerAbort(ctx, FAULT_EMERGENCY_STOP);
    }
}
