/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "cmsis_os.h"
#include "memorymap.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "stm32h7xx_nucleo.h"
#include "croutine.h"
#include "uart_dma.h"
#include "timerForDebug.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
extern UART_HandleTypeDef UartHandle;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);

static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleTaskStart = NULL;
uint32_t UserButtonStatus = 0;  /* set to 1 after User Button interrupt  */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
*********************************************************************************************************
* �? �? �?: vTaskTaskUserIF
* 功能说明: 接口消息处理，这里用�? LED 闪烁
* �? �?: pvParameters 是在创建该任务时传�?�的形参
* �? �? �?: �?
* �? �? �?: 1 (数�?�越小优先级越低，这个跟 uCOS 相反)
*********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters)
{
  
  uint8_t pcWriteBuffer[500];
  
  while(1)
  {
    
    
    if(UserButtonStatus == 1)
    {
      UserButtonStatus = 0;
      printf("============================================================\r\n");
      printf("TaskName TaskStatus Priority RemainStack TaskID\r\n");
      vTaskList((char *)pcWriteBuffer);
      printf("%s\r\n", pcWriteBuffer);
      
      printf("\r\nTaskName      RunCount       Usage\r\n");
      vTaskGetRunTimeStats((char *)pcWriteBuffer);
      printf("%s\r\n", pcWriteBuffer);
      
    }
    vTaskDelay(20);
  }
}

/*
*********************************************************************************************************
* �? �? �?: vTaskLED
* 功能说明: LED 闪烁
* �? �?: pvParameters 是在创建该任务时传�?�的形参
* �? �? �?: �?
* �? �? �?: 2
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
  while(1)
  {
    
    BSP_LED_On(LED2);
    vTaskDelay(1000);
    BSP_LED_Off(LED2);
    vTaskDelay(1000);
  }
}

/*
*********************************************************************************************************
* �? �? �?: vTaskMsgPro
* 功能说明: 信息处理
* �? �?: pvParameters 是在创建该任务时传�?�的形参
* �? �? �?: �?
* �? �? �?: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
  while(1)
  {
    BSP_LED_On(LED1);
    vTaskDelay(1000);
    BSP_LED_Off(LED1);
    vTaskDelay(1000);
    //printf("hello\r\n");
    vTaskDelay(1000);
  }
}

/*
*********************************************************************************************************
* �? �? �?: vTaskStart
* 功能说明: 启动任务，也就是�?高优先级任务，这里用�? LED 闪烁
* �? �?: pvParameters 是在创建该任务时传�?�的形参
* �? �? �?: �?
* �? �? �?: 4
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
  while(1)
  {
    
    BSP_LED_On(LED3);
    vTaskDelay(1000);
    BSP_LED_Off(LED3);
    vTaskDelay(1000);
  }
}

static void AppTaskCreate(void)
{
  xTaskCreate(vTaskTaskUserIF,          /*任务函数*/
              "vTaskUserIF",            /*任务�?*/
              512,                      /*stack大小，单位word，也就是4字节*/
              NULL,                     /*任务参数*/
              1,                        /*任务优先�?*/
              &xHandleTaskUserIF);      /*任务句柄*/
  
    xTaskCreate(vTaskLED,          /*任务函数*/
              "vTaskLED",              /*任务�?*/
              512,                       /*stack大小，单位word，也就是4字节*/
              NULL,                      /*任务参数*/
              2,                         /*任务优先�?*/
              &xHandleTaskLED);       /*任务句柄*/
    
      xTaskCreate(vTaskMsgPro,          /*任务函数*/
              "vTaskMsgPro",            /*任务�?*/
              512,                      /*stack大小，单位word，也就是4字节*/
              NULL,                     /*任务参数*/
              3,                        /*任务优先�?*/
              &xHandleTaskMsgPro);      /*任务句柄*/
      
        xTaskCreate(vTaskStart,          /*任务函数*/
              "vTaskStart",            /*任务�?*/
              512,                      /*stack大小，单位word，也就是4字节*/
              NULL,                     /*任务参数*/
              4,                        /*任务优先�?*/
              &xHandleTaskStart);      /*任务句柄*/
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  __set_PRIMASK(1);
  
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
  uartDmaInit();
  timer4DebugInit();
  /*创建任务*/
  AppTaskCreate();
  
  /*启动调度，开始执行任�?*/
  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Init scheduler */
  //osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  //MX_FREERTOS_Init();

  /* Start scheduler */
  //osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == BUTTON_USER_PIN)
  {  
    UserButtonStatus = 1;
  }
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
