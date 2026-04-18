#include "mission_fsm.h"

#include "mission.h"

void MissionFSM_Init(SystemContext_t *ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    ctx->state = MS_IDLE;
}

void MissionFSM_SetState(SystemContext_t *ctx, MissionState_e state)
{
    if (ctx == NULL)
    {
        return;
    }

    ctx->state = state;
}

void MissionFSM_Run(SystemContext_t *ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    switch (ctx->state)
    {
    case MS_IDLE:
        MissionFSM_SetState(ctx, MS_SELF_CHECK);
        break;

    case MS_SELF_CHECK:
        MissionFSM_SetState(ctx, MS_WAIT_TASK);
        break;

    case MS_WAIT_TASK:
        if (Mission_IsReady(&ctx->mission) != 0U)
        {
            MissionFSM_SetState(ctx, MS_WAIT_START);
        }
        break;

    case MS_WAIT_START:
        if (Mission_IsReady(&ctx->mission) == 0U)
        {
            MissionFSM_SetState(ctx, MS_WAIT_TASK);
        }
        else if (ctx->start_requested != 0U)
        {
            ctx->start_requested = 0U;
            MissionFSM_SetState(ctx, MS_TAKEOFF);
        }
        break;

    case MS_TAKEOFF:
        break;

    case MS_ABORT:
    case MS_ERROR:
    case MS_FINISH:
    default:
        break;
    }
}

const char *MissionFSM_StateName(MissionState_e state)
{
    switch (state)
    {
    case MS_IDLE: return "IDLE";
    case MS_SELF_CHECK: return "SELF_CHECK";
    case MS_WAIT_TASK: return "WAIT_TASK";
    case MS_WAIT_START: return "WAIT_START";
    case MS_TAKEOFF: return "TAKEOFF";
    case MS_GO_TO_WAYPOINT: return "GO_TO_WAYPOINT";
    case MS_HOVER_AND_CHECK: return "HOVER_AND_CHECK";
    case MS_TARGET_PROCESS: return "TARGET_PROCESS";
    case MS_NEXT_WAYPOINT: return "NEXT_WAYPOINT";
    case MS_RETURN_HOME: return "RETURN_HOME";
    case MS_PRE_LAND: return "PRE_LAND";
    case MS_LAND: return "LAND";
    case MS_FINISH: return "FINISH";
    case MS_ABORT: return "ABORT";
    case MS_ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}
