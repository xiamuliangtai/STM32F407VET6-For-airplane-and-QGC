#ifndef __MISSION_H
#define __MISSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"

void Mission_Init(Mission_t *mission);
void Mission_Clear(Mission_t *mission);
uint8_t Mission_LoadPath(Mission_t *mission, uint8_t seq, uint8_t point_count, const GsPoint_t *points);
uint8_t Mission_IsReady(const Mission_t *mission);
GsPoint_t *Mission_GetCurrentPoint(Mission_t *mission);
uint8_t Mission_Advance(Mission_t *mission);

#ifdef __cplusplus
}
#endif

#endif /* __MISSION_H */
