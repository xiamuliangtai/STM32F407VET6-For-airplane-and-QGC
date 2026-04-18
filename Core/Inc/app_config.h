#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_DEBUG_ENABLE                    1U

#define APP_MAX_WAYPOINTS                   32U

#define APP_UART_RX_FRAME_MAX_LEN           192U
#define APP_PROTO_MAX_PAYLOAD_LEN           128U
#define APP_PROTO_MAX_FRAME_LEN             (APP_PROTO_MAX_PAYLOAD_LEN + 7U)

#define APP_GS_TIMEOUT_MS                   2000U
#define APP_UWB_TIMEOUT_MS                  500U
#define APP_FC_TIMEOUT_MS                   500U
#define APP_VISION_TIMEOUT_MS               500U

#define APP_TELEMETRY_HEARTBEAT_MS          1000U
#define APP_TELEMETRY_STATE_MS              300U

#define APP_ARRIVE_XY_TH_M                  0.15f
#define APP_ARRIVE_Z_TH_M                   0.10f
#define APP_ARRIVE_HOLD_MS                  300U

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONFIG_H */
