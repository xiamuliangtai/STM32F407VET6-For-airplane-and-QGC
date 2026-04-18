#ifndef __MISSION_H
#define __MISSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"

void Mission_Init(Mission_t *mission);
void Mission_Clear(Mission_t *mission);
uint8_t Mission_SetInfo(Mission_t *mission, uint16_t task_id, uint16_t total_points);
uint8_t Mission_SetWaypoint(Mission_t *mission, uint16_t index, const WayPoint_t *wp);
uint8_t Mission_Commit(Mission_t *mission);
uint8_t Mission_IsReady(const Mission_t *mission);
WayPoint_t *Mission_GetCurrentWaypoint(Mission_t *mission);
uint8_t Mission_Advance(Mission_t *mission);

#ifdef __cplusplus
}
#endif

#endif /* __MISSION_H */
