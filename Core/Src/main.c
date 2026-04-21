/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_context.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "indicator.h"
#include "mission_fsm.h"
#include "oled.h"
#include "protocol.h"
#include "protocol_fc.h"
#include "protocol_gs.h"
#include "protocol_openmv.h"
#include "protocol_uwb.h"
#include "safety.h"
#include "telemetry.h"
#include "usart.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t s_oled_last_update_ms;
static uint8_t s_oled_ready;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void OLED_UpdateGsRxDisplay(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void OLED_UpdateGsRxDisplay(void)
{
  uint8_t rx_status;
  uint8_t rx_code;
  uint8_t rx_seq;
  uint8_t mission_valid;
  uint8_t mission_seq;
  uint8_t mission_points;
  uint32_t now_ms;

  if (s_oled_ready == 0U)
  {
    return;
  }

  now_ms = BSP_Timer_NowMs();
  rx_status = (uint8_t)g_app.gs_rx_status;
  rx_code = g_app.gs_last_rx_code;
  rx_seq = g_app.gs_last_rx_seq;
  mission_valid = g_app.mission.valid;
  mission_seq = g_app.mission.seq;
  mission_points = g_app.mission.point_count;

  if ((g_app.gs_last_rx_event_ms == 0U) ||
      (BSP_Timer_IsElapsed(now_ms, g_app.gs_last_rx_event_ms, APP_OLED_RX_HOLD_MS) != 0U))
  {
    rx_status = (uint8_t)GS_RX_STATUS_WAIT;
    rx_code = 0U;
    rx_seq = 0U;
    mission_valid = 0U;
    mission_seq = 0U;
    mission_points = 0U;
  }

  OLED_ShowGsRxScreen(g_app.gs_online,
                      rx_status,
                      rx_code,
                      rx_seq,
                      mission_valid,
                      mission_seq,
                      mission_points);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  MX_USART1_UART_Init();
  BSP_Timer_Init();
  BSP_UART_InitAll();
  AppContext_Init(&g_app);
  ProtocolGS_Init();
  ProtocolFC_Init();
  ProtocolUWB_Init();
  ProtocolOpenMV_Init();
  Safety_Init();
  Telemetry_Init();
  Indicator_Init();
  MissionFSM_Init(&g_app);
  s_oled_last_update_ms = HAL_GetTick();
  s_oled_ready = OLED_Init();
  OLED_UpdateGsRxDisplay();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    BSP_UART_PollFlags();
    Protocol_Dispatch();
    Safety_Check(&g_app);
    MissionFSM_Run(&g_app);
    Telemetry_Task(&g_app);
    Indicator_Update(&g_app);

    if ((s_oled_ready != 0U) && ((HAL_GetTick() - s_oled_last_update_ms) >= 500U))
    {
      s_oled_last_update_ms = HAL_GetTick();
      OLED_UpdateGsRxDisplay();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
