#include "bsp_uart.h"

#include <string.h>

#include "usart.h"

typedef struct
{
    UartRxFrame_t frame;
    uint8_t pending;
} UartMailbox_t;

static UartMailbox_t s_rx_mailbox[UART_PORT_COUNT];
static uint8_t s_rx_dma_buf[UART_PORT_COUNT][APP_UART_RX_FRAME_MAX_LEN];

static UART_HandleTypeDef *uart_handle_from_port(UartPort_e port)
{
    switch (port)
    {
    case UART_PORT_GS:
        return &huart1;

    default:
        return NULL;
    }
}

static DMA_HandleTypeDef *uart_rx_dma_from_port(UartPort_e port)
{
    switch (port)
    {
    case UART_PORT_GS:
        return &hdma_usart1_rx;

    default:
        return NULL;
    }
}

static UartPort_e uart_port_from_handle(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        return UART_PORT_GS;
    }

    return UART_PORT_COUNT;
}

void BSP_UART_InitAll(void)
{
    memset(s_rx_mailbox, 0, sizeof(s_rx_mailbox));
    memset(s_rx_dma_buf, 0, sizeof(s_rx_dma_buf));

    BSP_UART_StartRx(UART_PORT_GS);
}

void BSP_UART_StartRx(UartPort_e port)
{
    UART_HandleTypeDef *huart = uart_handle_from_port(port);
    DMA_HandleTypeDef *hdma = uart_rx_dma_from_port(port);

    if (huart == NULL)
    {
        return;
    }

    if (HAL_UARTEx_ReceiveToIdle_DMA(huart, s_rx_dma_buf[port], APP_UART_RX_FRAME_MAX_LEN) == HAL_OK)
    {
        if (hdma != NULL)
        {
            __HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT);
        }
    }
}

HAL_StatusTypeDef BSP_UART_Send(UartPort_e port, const uint8_t *data, uint16_t len)
{
    UART_HandleTypeDef *huart = uart_handle_from_port(port);

    if ((huart == NULL) || (data == NULL) || (len == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_UART_Transmit(huart, (uint8_t *)data, len, 100U);
}

uint8_t BSP_UART_FetchFrame(UartPort_e port, UartRxFrame_t *frame)
{
    if ((port >= UART_PORT_COUNT) || (frame == NULL) || (s_rx_mailbox[port].pending == 0U))
    {
        return 0U;
    }

    memcpy(frame, &s_rx_mailbox[port].frame, sizeof(*frame));
    s_rx_mailbox[port].pending = 0U;
    return 1U;
}

uint8_t BSP_UART_PushRxFrame(UartPort_e port, const uint8_t *data, uint16_t len)
{
    if ((port >= UART_PORT_COUNT) || (data == NULL) || (len == 0U) || (len > APP_UART_RX_FRAME_MAX_LEN))
    {
        return 0U;
    }

    memcpy(s_rx_mailbox[port].frame.data, data, len);
    s_rx_mailbox[port].frame.len = len;
    s_rx_mailbox[port].pending = 1U;
    return 1U;
}

void BSP_UART_PollFlags(void)
{
}

void BSP_UART_IdleCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    UartPort_e port = uart_port_from_handle(huart);

    if ((port < UART_PORT_COUNT) && (Size > 0U) && (Size <= APP_UART_RX_FRAME_MAX_LEN))
    {
        (void)BSP_UART_PushRxFrame(port, s_rx_dma_buf[port], Size);
        BSP_UART_StartRx(port);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    UartPort_e port = uart_port_from_handle(huart);

    if (port < UART_PORT_COUNT)
    {
        (void)HAL_UART_DMAStop(huart);
        BSP_UART_StartRx(port);
    }
}
