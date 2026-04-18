#ifndef __TELEMETRY_H
#define __TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"

void Telemetry_Init(void);
void Telemetry_ReportHeartbeat(SystemContext_t *ctx);
void Telemetry_ReportState(SystemContext_t *ctx);
void Telemetry_ReportFault(SystemContext_t *ctx);
void Telemetry_Task(SystemContext_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* __TELEMETRY_H */
