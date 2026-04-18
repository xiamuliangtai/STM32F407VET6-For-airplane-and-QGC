#include "app_context.h"

#include <string.h>

SystemContext_t g_app;

void AppContext_Init(SystemContext_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->state = MS_IDLE;
    ctx->fault_code = FAULT_NONE;
}
