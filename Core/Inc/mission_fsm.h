#ifndef __MISSION_FSM_H
#define __MISSION_FSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"

void MissionFSM_Init(SystemContext_t *ctx);
void MissionFSM_Run(SystemContext_t *ctx);
void MissionFSM_SetState(SystemContext_t *ctx, MissionState_e state);
const char *MissionFSM_StateName(MissionState_e state);

#ifdef __cplusplus
}
#endif

#endif /* __MISSION_FSM_H */
