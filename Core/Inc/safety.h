#ifndef __SAFETY_H
#define __SAFETY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"

void Safety_Init(void);
void Safety_Check(SystemContext_t *ctx);
void Safety_TriggerAbort(SystemContext_t *ctx, FaultCode_e fault);
uint8_t Safety_IsMissionAllowed(const SystemContext_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* __SAFETY_H */
