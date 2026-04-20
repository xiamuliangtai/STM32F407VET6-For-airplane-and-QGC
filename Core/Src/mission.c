#include "mission.h"

#include <string.h>

void Mission_Init(Mission_t *mission)
{
    Mission_Clear(mission);
}

void Mission_Clear(Mission_t *mission)
{
    if (mission == NULL)
    {
        return;
    }

    memset(mission, 0, sizeof(*mission));
}

uint8_t Mission_LoadPath(Mission_t *mission, uint8_t seq, uint8_t point_count, const GsPoint_t *points)
{
    if ((mission == NULL) || (points == NULL) || (point_count == 0U) || (point_count > APP_MAX_WAYPOINTS))
    {
        return 0U;
    }

    Mission_Clear(mission);
    mission->seq = seq;
    mission->point_count = point_count;
    memcpy(mission->points, points, (uint32_t)point_count * sizeof(GsPoint_t));
    mission->current_index = 0U;
    mission->valid = 1U;
    return 1U;
}

uint8_t Mission_IsReady(const Mission_t *mission)
{
    return ((mission != NULL) && (mission->valid != 0U) && (mission->point_count > 0U)) ? 1U : 0U;
}

GsPoint_t *Mission_GetCurrentPoint(Mission_t *mission)
{
    if ((mission == NULL) || (mission->valid == 0U) || (mission->current_index >= mission->point_count))
    {
        return NULL;
    }

    return &mission->points[mission->current_index];
}

uint8_t Mission_Advance(Mission_t *mission)
{
    if ((mission == NULL) || (mission->valid == 0U))
    {
        return 0U;
    }

    if ((mission->current_index + 1U) >= mission->point_count)
    {
        return 0U;
    }

    mission->current_index++;
    return 1U;
}
