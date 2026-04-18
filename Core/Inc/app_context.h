#ifndef __APP_CONTEXT_H
#define __APP_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"

extern SystemContext_t g_app;

void AppContext_Init(SystemContext_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONTEXT_H */
