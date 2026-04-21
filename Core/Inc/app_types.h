#ifndef __APP_TYPES_H
#define __APP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_config.h"

typedef enum
{
    WP_NORMAL = 0,
    WP_CHECK,
    WP_HOME,
    WP_PRELAND,
    WP_LAND
} WaypointType_e;

typedef enum
{
    MS_IDLE = 0,
    MS_SELF_CHECK,
    MS_WAIT_TASK,
    MS_WAIT_START,
    MS_TAKEOFF,
    MS_GO_TO_WAYPOINT,
    MS_HOVER_AND_CHECK,
    MS_TARGET_PROCESS,
    MS_NEXT_WAYPOINT,
    MS_RETURN_HOME,
    MS_PRE_LAND,
    MS_LAND,
    MS_FINISH,
    MS_ABORT,
    MS_ERROR
} MissionState_e;

typedef enum
{
    FAULT_NONE = 0,
    FAULT_PROTOCOL = 1,
    FAULT_TASK_INVALID = 2,
    FAULT_EMERGENCY_STOP = 3,
    FAULT_MISSION_ABORT = 4
} FaultCode_e;

typedef enum
{
    GS_RX_STATUS_WAIT = 0,
    GS_RX_STATUS_OK,
    GS_RX_STATUS_ERR
} GsRxStatus_e;

typedef struct
{
    float x;
    float y;
    float z;
    uint8_t type;
    uint16_t hold_ms;
} WayPoint_t;

typedef struct
{
    uint8_t col;
    uint8_t row;
    uint8_t type;
    uint16_t hold_ms;
    uint8_t reserved;
} GsPoint_t;

typedef struct
{
    uint8_t seq;
    uint8_t point_count;
    uint8_t current_index;
    uint8_t valid;
    GsPoint_t points[APP_MAX_WAYPOINTS];
} Mission_t;

typedef struct
{
    uint8_t valid;
    float x;
    float y;
    float z;
    uint16_t quality;
    uint32_t tick;
} UwbData_t;

typedef struct
{
    uint8_t valid;
    uint8_t cls;
    uint8_t count;
    int16_t dx;
    int16_t dy;
    uint16_t score;
    uint32_t tick;
} VisionData_t;

typedef struct
{
    uint8_t online;
    uint8_t armed;
    uint8_t mode;
    float x;
    float y;
    float z;
    float yaw;
    float voltage;
    uint32_t tick;
} FcState_t;

typedef struct
{
    Mission_t mission;
    MissionState_e state;

    UwbData_t uwb;
    VisionData_t vision;
    FcState_t fc;

    uint32_t now_ms;
    uint32_t last_gs_rx_ms;
    uint32_t last_uwb_rx_ms;
    uint32_t last_fc_rx_ms;
    uint32_t last_vision_rx_ms;

    uint8_t gs_online;
    uint8_t uwb_online;
    uint8_t fc_online;
    uint8_t vision_online;
    GsRxStatus_e gs_rx_status;
    uint32_t gs_last_rx_event_ms;
    uint8_t gs_last_rx_code;
    uint8_t gs_last_rx_seq;

    uint8_t emergency_stop;
    uint8_t mission_abort;
    uint8_t start_requested;
    FaultCode_e fault_code;
} SystemContext_t;

#ifdef __cplusplus
}
#endif

#endif /* __APP_TYPES_H */
