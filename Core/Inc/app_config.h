#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_DEBUG_ENABLE                    1U

#define APP_MAX_WAYPOINTS                   64U

#define APP_GS_UART_BAUDRATE                500000U

#define APP_UART_RX_FRAME_MAX_LEN           512U
#define APP_PROTO_MAX_PAYLOAD_LEN           (1U + (APP_MAX_WAYPOINTS * 6U))
#define APP_PROTO_MAX_FRAME_LEN             (APP_PROTO_MAX_PAYLOAD_LEN + 8U)

#define APP_GS_TIMEOUT_MS                   2000U
#define APP_OLED_RX_HOLD_MS                 5000U
#define APP_ANIMAL_REPORT_MIN_INTERVAL_MS   1000U
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
