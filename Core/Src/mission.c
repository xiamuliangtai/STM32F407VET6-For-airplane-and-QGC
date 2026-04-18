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

uint8_t Mission_SetInfo(Mission_t *mission, uint16_t task_id, uint16_t total_points)
{
    if ((mission == NULL) || (total_points == 0U) || (total_points > APP_MAX_WAYPOINTS))
    {
        return 0U;
    }

    Mission_Clear(mission);
    mission->task_id = task_id;
    mission->total_points = total_points;
    return 1U;
}

uint8_t Mission_SetWaypoint(Mission_t *mission, uint16_t index, const WayPoint_t *wp)
{
    if ((mission == NULL) || (wp == NULL) || (mission->total_points == 0U) || (index >= mission->total_points))
    {
        return 0U;
    }

    mission->points[index] = *wp;
    if (mission->point_written[index] == 0U)
    {
        mission->point_written[index] = 1U;
        mission->uploaded_points++;
    }

    return 1U;
}

uint8_t Mission_Commit(Mission_t *mission)
{
    uint16_t i;

    if ((mission == NULL) || (mission->total_points == 0U))
    {
        return 0U;
    }

    for (i = 0U; i < mission->total_points; i++)
    {
        if (mission->point_written[i] == 0U)
        {
            mission->valid = 0U;
            return 0U;
        }
    }

    mission->valid = 1U;
    mission->current_index = 0U;
    return 1U;
}

uint8_t Mission_IsReady(const Mission_t *mission)
{
    return ((mission != NULL) && (mission->valid != 0U) && (mission->total_points > 0U)) ? 1U : 0U;
}

WayPoint_t *Mission_GetCurrentWaypoint(Mission_t *mission)
{
    if ((mission == NULL) || (mission->valid == 0U) || (mission->current_index >= mission->total_points))
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

    if ((mission->current_index + 1U) >= mission->total_points)
    {
        return 0U;
    }

    mission->current_index++;
    return 1U;
}
