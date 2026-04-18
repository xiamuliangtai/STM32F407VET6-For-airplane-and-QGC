#ifndef __BSP_UART_H
#define __BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "app_config.h"

typedef enum
{
    UART_PORT_GS = 0,
    UART_PORT_FC,
    UART_PORT_UWB,
    UART_PORT_OPENMV,
    UART_PORT_DEBUG,
    UART_PORT_COUNT
} UartPort_e;

typedef struct
{
    uint8_t data[APP_UART_RX_FRAME_MAX_LEN];
    uint16_t len;
} UartRxFrame_t;

void BSP_UART_InitAll(void);
void BSP_UART_StartRx(UartPort_e port);
HAL_StatusTypeDef BSP_UART_Send(UartPort_e port, const uint8_t *data, uint16_t len);
uint8_t BSP_UART_FetchFrame(UartPort_e port, UartRxFrame_t *frame);
uint8_t BSP_UART_PushRxFrame(UartPort_e port, const uint8_t *data, uint16_t len);
void BSP_UART_PollFlags(void);
void BSP_UART_IdleCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_UART_H */
