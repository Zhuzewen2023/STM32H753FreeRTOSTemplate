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
#include "dma.h"
#include "memorymap.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "stm32h7xx_nucleo.h"
#include "croutine.h"
#include "timerForDebug.h"
#include "iwdg.h"
#include "wwdg.h"
#include "delay.h"
#include "rtc_timeStamp.h"
#include "bootLoader.h"
#include "cpu_flash.h"
#include "queue.h"
#include "stm32h7xx.h"
#include "event_groups.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* 定义�?512KB AXI SRAM里面的变�? */
#pragma location = ".RAM_D1"  
uint32_t AXISRAMBuf[10];
#pragma location = ".RAM_D1"  
uint16_t AXISRAMCount;

/* 定义�?128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)里面的变�? */
#pragma location = ".RAM_D2" 
uint32_t D2SRAMBuf[10];
#pragma location = ".RAM_D2" 
uint16_t D2SRAMCount;

/* 定义�?64KB SRAM4(0x38000000)里面的变�? */
#pragma location = ".RAM_D3"  
uint32_t D3SRAMBuf[10];
#pragma location = ".RAM_D3"  
uint16_t D3SRAMCount;

#pragma location = ".RAM_BKP"
uint32_t BKPSRAMBuf[10];
#pragma location = ".RAM_BKP"
uint16_t BKPSRAMCount;

/* 
   1、将�?个扇区的空间预留出来做为参数区，这里是将�?2个扇区作为参数区�?
      默认情况下不要将�?1个扇区做参数区，因为�?1个扇区是默认的boot启动地址�?
   2、�?�过这种定义方式告诉编译器，此空间已经被占用，不让编译器再为这个空间编写程序�?
*/
#pragma location=0x08000000 + 128*1024
const uint8_t para_flash_area[128 * 1024];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
extern IWDG_HandleTypeDef IwdgHandle;
extern WWDG_HandleTypeDef   WwdgHandle;
extern uint32_t wwdgDelayNum;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef struct
{
  uint8_t ParamVer;
  uint16_t ucBackLight;
  uint32_t Baud485;
  float ucRadioMode;
}PARAM_T;
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
static void vTaskIwdg(void *pvParameters);
static void vTaskWwdg(void *pvParameters);
//static void vTaskRtcTimeStamp(void *pvParameters);

static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleTaskStart = NULL;
static TaskHandle_t xHandleTaskIwdg = NULL;
static TaskHandle_t xHandleTaskWwdg = NULL;
//static TaskHandle_t xHandleTaskRtcTimeStamp = NULL;

EventGroupHandle_t xHandleEventGroup;

QueueHandle_t xQueue1 = NULL;
QueueHandle_t xQueue2 = NULL;

typedef struct Msg
{
  uint8_t ucMessageID;
  uint16_t usData[2];
  uint32_t ulData[2];
}MSG_T;

MSG_T   g_tMsg;

uint32_t UserButtonStatus = 0;  /* set to 1 after User Button interrupt  */

TimerHandle_t xTimers[2];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void vTimerCallback(xTimerHandle pxTimer)
{
    uint32_t ulTimerID;
    
    ulTimerID = (uint32_t)pvTimerGetTimerID(pxTimer);
    
    if(ulTimerID == 0)
    {
      printf("software timer 0\r\n");
    }
    
    if(ulTimerID == 1)
    {
      printf("software timer 1\r\n");
    }
}

static void AppObjCreate(void)
{
  /*创建10个uint8_t型的消息队列*/
  xQueue1 = xQueueCreate(10, sizeof(uint8_t));
  if(xQueue1 == NULL)
  {
    printf("create xQueue1 failed\r\n");
  }
  else
  {
    printf("create xQueue1 success\r\n");
  }
  /*创建10个存储指针变量的消息队列*/
  xQueue2 = xQueueCreate(10, sizeof(struct Msg *));
  if(xQueue2 == NULL)
  {
    printf("create xQueue2 failed\r\n");
  }
  else
  {
    printf("create xQueue2 success\r\n");
  }
  /*创建软件定时器*/
  uint8_t i;
  const TickType_t xTimerPer[2] = {1000, 1000};
  
  for(i = 0; i < 2; i++)
  {
    xTimers[i] = xTimerCreate("Timer",                  /*定时器名字*/ 
                              xTimerPer[i],             /*定时器周期，单位时钟节拍*/ 
                              pdTRUE,                   /*周期性*/
                              (void *)i,                /*定时器ID*/
                              vTimerCallback);          /*定时器回调函数*/
    
    if(xTimers[i] == NULL)
    {
      printf("create software timer failed , ID = %d\r\n", i);
    }
    else
    {
      if(xTimerStart(xTimers[i], 100) != pdPASS)
      {
        printf("timer start failed, ID = %d\r\n", i);
      }
    }
  }
  /*创建事件标志组*/
  xHandleEventGroup = xEventGroupCreate();
  
  if(xHandleEventGroup == NULL)
  {
    /*事件标志组没有创建成功*/
    printf("xEventGroupCreate failed\r\n");
  }
}


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
  EventBits_t uxBits;
  
  uint8_t ucCount = 0;
  
  MSG_T *ptMsg;
  
  ptMsg = &g_tMsg;
  ptMsg->ucMessageID = 0;
  ptMsg->ulData[0] = 0;
  ptMsg->usData[0] = 0;
  //uint8_t buff[2048];
  
  while(1)
  {
    
    
    if(UserButtonStatus == 1)
    {
      //UserButtonStatus = 0;
      printf("============================================================\r\n");
      printf("TaskName TaskStatus Priority RemainStack TaskID\r\n");
      vTaskList((char *)pcWriteBuffer);
      printf("%s\r\n", pcWriteBuffer);
      
      printf("\r\nTaskName      RunCount       Usage\r\n");
      vTaskGetRunTimeStats((char *)pcWriteBuffer);
      printf("%s\r\n", pcWriteBuffer);
      
      RTC_CalendarShow();
      
      AXISRAMBuf[0] = AXISRAMCount++;
      AXISRAMBuf[5] = AXISRAMCount++;
      AXISRAMBuf[9] = AXISRAMCount++;
      printf("UserButtonStatus = 1, AXISRAMBuf[0] = %d, AXISRAMBuf[5] = %d, AXISRAMBuf[9] = %d\r\n",AXISRAMBuf[0], AXISRAMBuf[5], AXISRAMBuf[9]);
      
      
    }
    else if(UserButtonStatus == 2)
    {
      printf("UserButtonStatus = 2\r\ndelete led task\r\n");
      if(xHandleTaskLED != NULL)
      {
        vTaskDelete(xHandleTaskLED);
        xHandleTaskLED = NULL;
      }
      
      D2SRAMBuf[0] = D2SRAMCount++;
      D2SRAMBuf[5] = D2SRAMCount++;
      D2SRAMBuf[9] = D2SRAMCount++;
      printf("UserButtonStatus = 2, D2SRAMBuf[0] = %d, D2SRAMBuf[5] = %d, D2SRAMBuf[9] = %d\r\n",D2SRAMBuf[0], D2SRAMBuf[5], D2SRAMBuf[9]);
    }
    else if(UserButtonStatus == 3)
    {
      printf("UserButtonStatus = 3\r\ncreate led task\r\n");
      if(xHandleTaskLED == NULL)
      {
        xTaskCreate(  vTaskLED,
                        "vTaskLED",
                        512,
                        NULL,
                        2,
                        &xHandleTaskLED);
      }
      D3SRAMBuf[0] = D3SRAMCount++;
      D3SRAMBuf[5] = D3SRAMCount++;
      D3SRAMBuf[9] = D3SRAMCount++;
      printf("UserButtonStatus = 3, D3SRAMBuf[0] = %d, D3SRAMBuf[5] = %d, D3SRAMBuf[9] = %d\r\n",D3SRAMBuf[0], D3SRAMBuf[5], D3SRAMBuf[9]);
    }
    else if(UserButtonStatus == 4)
    {
      taskENTER_CRITICAL();
      printf("UserButtonStatus = 4\r\nsuspend led task\r\n");
      vTaskSuspend(xHandleTaskLED);
      BKPSRAMBuf[0] = BKPSRAMCount++;
      BKPSRAMBuf[5] = BKPSRAMCount++;
      BKPSRAMBuf[9] = BKPSRAMCount++;
      printf("UserButtonStatus = 3, BKPSRAMBuf[0] = %d, BKPSRAMBuf[5] = %d, BKPSRAMBuf[9] = %d\r\n",BKPSRAMBuf[0], BKPSRAMBuf[5], BKPSRAMBuf[9]);
      taskEXIT_CRITICAL();
      
    }
    else if(UserButtonStatus == 5)
    {
      printf("UserButtonStatus = 5\r\nresume led task\r\n");
      HAL_TIM_Base_Start_IT(&htim17);
      //vTaskResume(xHandleTaskLED);
    }
    else if(UserButtonStatus == 6)
    {
      /*
      uxBits = xEventGroupSetBits(xHandleEventGroup, SET_BIT_0);
      if((uxBits & SET_BIT_0) != 0)
      {
        printf("UserButtonStatus = 6, event group bit 0 was setted\r\n");
      }
      else
      {
        printf("UserButtonStatus = 6, event group bit 0 was not setted\r\n");
      }
      */
      //printf("UserButtonStatus = 6\r\nsuspend iwdg feed task\r\n");
      //vTaskSuspend(xHandleTaskIwdg);
    }
    else if(UserButtonStatus == 7)
    {
      /*
      uxBits = xEventGroupSetBits(xHandleEventGroup, SET_BIT_1);
      if((uxBits & SET_BIT_1) != 0)
      {
        printf("UserButtonStatus = 7, event group bit 1 was setted\r\n");
      }
      else
      {
        printf("UserButtonStatus = 7, event group bit 1 was not setted\r\n");
      }
      */
      vTaskResume(xHandleTaskStart);
      //printf("UserButtonStatus = 7\r\nresume iwdg feed task\r\n");
      //vTaskResume(xHandleTaskIwdg);
    }
    else if(UserButtonStatus == 8)
    {
      vTaskSuspendAll(); /*开启调度锁*/
      uxBits = xEventGroupSetBits(xHandleEventGroup, SET_BIT_0);
      if((uxBits & SET_BIT_0) != 0)
      {
        printf("UserButtonStatus = 8, event group bit 0 was setted\r\n");
      }
      else
      {
        printf("UserButtonStatus = 8, event group bit 0 was not setted\r\n");
      }
      uxBits = xEventGroupSetBits(xHandleEventGroup, SET_BIT_1);
      if((uxBits & SET_BIT_1) != 0)
      {
        printf("UserButtonStatus = 8, event group bit 1 was setted\r\n");
      }
      else
      {
        printf("UserButtonStatus = 8, event group bit 1 was not setted\r\n");
      }
      if(xTaskResumeAll() == pdTRUE) /*关闭调度锁，如果需要任务切换，此函数返回pdTRUE*/
      {
        taskYIELD();
      }
      //printf("UserButtonStatus = 8\r\nsuspend wwdg feed task\r\n");
      //vTaskSuspend(xHandleTaskWwdg);
    }
    else if(UserButtonStatus == 9)
    {
      ucCount++;
      if(xQueueSend(xQueue1,
                    (void *) &ucCount,
                    (TickType_t)10) != pdPASS)
      {
        /*发送失败*/
        printf("UserButtonStatus = 9, xQueue1Send ucCount failed\r\n");
      }
      else
      {
        printf("UserButtonStatus = 9, xQueue1Send ucCount success\r\n");
      }
      printf("UserButtonStatus = 9, set vTaskLED priority to 5\r\n");
      vTaskPrioritySet(xHandleTaskLED, 5);
      printf("vTaskLED priority is now setted to %d\r\n", (int)uxTaskPriorityGet(xHandleTaskLED));
      
      printf("UserButtonStatus = 9, start tim15 for sending event\r\n");
      HAL_TIM_Base_Start_IT(&htim15);
      //printf("UserButtonStatus = 9\r\nresume wwdg feed task\r\n");
      //vTaskResume(xHandleTaskWwdg);
    }
    else if(UserButtonStatus == 10)
    {
      ptMsg->ucMessageID++;
      ptMsg->ulData[0]++;
      ptMsg->usData[0]++;
      
      if(xQueueSend(xQueue2, 
                    (void *)&ptMsg,
                    (TickType_t)10) != pdPASS)
      {
        printf("UserButtonStatus = 10, xQueue2Send MSG_T failed\r\n");
      }
      else
      {
        printf("UserButtonStatus = 10, xQueue2Send MSG_T success\r\n");
      }
      printf("UserButtonStatus = 10, set vTaskLED priority to 1\r\n");
      vTaskPrioritySet(xHandleTaskLED, 1);
      printf("vTaskLED priority is now setted to %d\r\n", (int)uxTaskPriorityGet(xHandleTaskLED));
      
      printf("UserButtonStatus = 10, start tim16 for sending event\r\n");
      HAL_TIM_Base_Start_IT(&htim16);
    }
    else if(UserButtonStatus == 11)
    {
      printf("UserButtonStatus = 11\r\nJumpToBootloader\r\n");
      JumpToBootLoader();
      
    }
    else
    {
      UserButtonStatus = 0;
      printf("UserButtonStatus reset to 0\r\n");
    }
    vTaskSuspend(xHandleTaskUserIF);
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
  MSG_T *ptMsg;
  BaseType_t xResult;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
  
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 1000;
  
  xLastWakeTime = xTaskGetTickCount();
  
  
  while(1)
  {
    //vTaskSuspendAll(); /*开启调度锁*/
    
    xResult = xQueueReceive(xQueue2,
                            (void *)&ptMsg,
                            (TickType_t)xMaxBlockTime);
    
    if(xResult == pdPASS)
    {
      printf("msg queue receive success: ptMsg->ucMessageID = %d\r\n", ptMsg->ucMessageID);
      printf("msg queue receive success: ptMsg->usData[0] = %d\r\n", ptMsg->usData[0]);
      printf("msg queue receive success: ptMsg->ulData[0] = %d\r\n", ptMsg->ulData[0]);
      
    }
    else
    {
      printf("receive queue2 msg failed\r\n");
    }
    
    xResult = xQueueReceive(xQueue2,
                            (void *)&ptMsg,
                            (TickType_t)xMaxBlockTime);
    
    if(xResult == pdPASS)
    {
      printf("======================================================================================\r\n");
      printf("msg queue receive success: ptMsg->ucMessageID = %d\r\n", ptMsg->ucMessageID);
      printf("msg queue receive success: ptMsg->usData[1] = %d\r\n", ptMsg->usData[1]);
      printf("msg queue receive success: ptMsg->ulData[1] = %d\r\n", ptMsg->ulData[1]);
    }
    else
    {
      printf("receive queue2 msg failed\r\n");
    }
    
    printf("now suspend all, vTaskLED working\r\n");
    printf("xLastWakeTime = %d\r\n", xLastWakeTime);
    BSP_LED_Toggle(LED1);
    //BSP_LED_Toggle(LED2);
    //BSP_LED_Toggle(LED3);
    RTC_CalendarShow();
    
#if 0
    if(xTaskResumeAll() == pdTRUE) /*关闭调度锁，如果需要任务切换，此函数返回pdTRUE*/
    {
      taskYIELD();
    }
    printf("after vTaskResumeAll\r\n");
#endif
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    
    
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
  BaseType_t xResult;
  const portTickType xMaxBlockTime = pdMS_TO_TICKS(300); /*设置最大等待时间为300ms*/
  uint8_t ucQueueMsgValue;
  uint8_t uiQueueMsgValue;
  
  static uint8_t frameBuffer[101] = {0};
  memset((char *)(&frameBuffer[1]), '2', 100);
  HAL_UART_Transmit_DMA(&huart6, "uart6 dma transmit success\r\n", strlen("uart6 dma transmit success\r\n"));
  //static uint8_t rcvBuf[5];
  //HAL_UART_Receive_DMA(&huart6, rcvBuf, 5);
  /* Invalidate cache prior to access by CPU */
  //SCB_InvalidateDCache_by_Addr ((uint32_t *)rcvBuf, 5);
  while(1)
  {
    xResult = xQueueReceive(xQueue1,
                            (void *)&ucQueueMsgValue,
                            (TickType_t)xMaxBlockTime);
    
    if(xResult == pdPASS)
    {
      printf("receive queue1 msg success\r\n");
      printf("ucQueueMsgValue = %d\r\n", ucQueueMsgValue);
    }
    else
    {
      printf("receive queue1 msg failed\r\n");
    }
    
    xResult = xQueueReceive(xQueue1, (void *)&uiQueueMsgValue, (TickType_t)xMaxBlockTime);
    
    if(xResult == pdPASS)
    {
      printf("receive queue1 msg success\r\n");
      printf("uiQueueMsgValue = %d\r\n", uiQueueMsgValue);
    }
    else
    {
      printf("receive queue1 msg failed\r\n");
    }
    
    //HAL_UART_Transmit_DMA(&huart7, "uart7 dma transmit success\r\n", strlen("uart6 dma transmit success\r\n"));
    uart7ModeChange(GPIOMODE);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
    delay_us(88);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);
    delay_us(8);
    uart7ModeChange(UARTMODE);
    {
      HAL_UART_Transmit(&huart7, frameBuffer, 100, 0xff);
    }
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
  //uartDmaTest();
  uint8_t ucTest, *ptr8;
  uint16_t uiTest, *ptr16;
  uint32_t ulTest, *ptr32;
  PARAM_T tPara, *paraPtr;
  
  tPara.Baud485 = 0x5555AAAA;
  tPara.ParamVer = 0x99;
  tPara.ucBackLight = 0x7788;
  tPara.ucRadioMode = 99.99f;
  
  while(1)
  {
    if(UserButtonStatus == 6)
    {
      taskDISABLE_INTERRUPTS();
      eraseCpuFlash((uint32_t)para_flash_area);
      
      ucTest = 0xAA;
      uiTest = 0x55AA;
      ulTest = 0x11223344;
      
      writeCpuFlash((uint32_t)para_flash_area, (uint8_t *)&ucTest, sizeof(ucTest));
      writeCpuFlash((uint32_t)para_flash_area + 32, (uint8_t *)&uiTest, sizeof(uiTest));
      writeCpuFlash((uint32_t)para_flash_area + 32 * 2, (uint8_t *)&ulTest, sizeof(ulTest));

      /* 读出数据并打�? */
      ptr8  = (uint8_t  *)(para_flash_area + 32*0);
      ptr16 = (uint16_t *)(para_flash_area + 32*1);
      ptr32 = (uint32_t *)(para_flash_area + 32*2);

      taskENABLE_INTERRUPTS();
      printf("userButtonStatus = 6\r\n");
      printf("write data: ucTest = %x, uiTest = %x, ulTest = %x\r\n", ucTest, uiTest, ulTest);
      printf("read data: ptr8 = %x, ptr16 = %x, ptr32 = %x\r\n", *ptr8, *ptr16, *ptr32);
      vTaskSuspend(xHandleTaskStart);
    }
    else if(UserButtonStatus == 7)
    {
      printf("userButtonStatus = 7\r\n");
      /* 擦除扇区 */
      eraseCpuFlash((uint32_t)para_flash_area);
      writeCpuFlash((uint32_t)para_flash_area, (uint8_t *)&tPara, sizeof(tPara));
      
      paraPtr = (PARAM_T *)((uint32_t)para_flash_area);
      
      printf("write data: Baud485=%x, ParamVer=%x, ucBackLight=%x, ucRadioMode=%f\r\n", 
              tPara.Baud485,
              tPara.ParamVer,
              tPara.ucBackLight,
              tPara.ucRadioMode);
				
      printf("read data: Baud485=%x, ParamVer=%x, ucBackLight=%x, ucRadioMode=%f\r\n", 
              paraPtr->Baud485,
              paraPtr->ParamVer,
              paraPtr->ucBackLight,
              paraPtr->ucRadioMode);
      vTaskSuspend(xHandleTaskStart);
    }
    
    vTaskDelay(10);
  }
}

static void vTaskIwdg(void *pvParameters)
{
  /*利用vTaskDelay实现vTaskDelayUntil*/
  TickType_t xDelay, xNextTime;
  const TickType_t xFrequency =  1000;
  
  /*获取第一次唤醒时间*/
  xNextTime = xTaskGetTickCount() + xFrequency;
  
  while(1)
  {
    printf("use vTaskDelay to achieve vTaskDelayUntil(1000)\r\n");
    xDelay = xNextTime - xTaskGetTickCount();
    //计算下次唤醒时间
    xNextTime += xFrequency;
    
    if(xDelay <= xFrequency)
    {
      vTaskDelay(xDelay);
    }
  }
#if 0
    if(HAL_IWDG_Refresh(&IwdgHandle) != HAL_OK)
    {
      printf("iwdg feed error\r\n");
    }
    else
    {
      //printf("iwdg feed success\r\n");
    }
#endif
    
  
}

static void vTaskWwdg(void *pvParameters)
{
  EventBits_t uxBits;
  const TickType_t xTicksToWait = 100  / portTICK_PERIOD_MS;  /*100ms*/
  while(1)
  {
    uxBits = xEventGroupWaitBits(xHandleEventGroup, 
                                 SET_BIT_0 | SET_BIT_1, /*等待bit0和bit1被设置*/
                                 pdTRUE,                /*退出前bit0和bit1都被清除*/
                                 pdTRUE,                /*等待bit0和bit1都被设置*/
                                 xTicksToWait);         /*等待延迟时间*/
    
    if((uxBits & (SET_BIT_0 | SET_BIT_1)) == (SET_BIT_0 | SET_BIT_1))
    {
      printf("receive event group bits: bit 0 and bit 1 was setted\r\n");
      BSP_LED_Toggle(LED2);
    }
    else
    {
      //printf("run overtime or bit not setted\r\n");
      //BSP_LED_Toggle(LED3);
    }
                                 
    vTaskDelay(1);
#if 0
    vTaskDelay(wwdgDelayNum);
    if(HAL_WWDG_Refresh(&WwdgHandle) != HAL_OK)
    {
      printf("Wwdg feed error\r\n");
    }
    else
    {
      //printf("Wwdg feed success\r\n");
    }
#endif
    
  }
}

/*
static void vTaskRtcTimeStamp(void *pvParameters)
{
  while(1)
  {
    RTC_CalendarShow();
    vTaskDelay(1000);
  }
}
*/

static void AppTaskCreate(void)
{
  xTaskCreate(vTaskTaskUserIF,          /*任务函数*/
              "vTaskUserIF",            /*任务�?*/
              512,                      /*stack大小，单位word，也就是4字节*/
              NULL,                     /*任务参数*/
              5,                        /*任务优先�?*/
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
  
#if 1
    xTaskCreate(vTaskIwdg,          /*任务函数*/
              "vTaskIwdg",            /*任务�?*/
              512,                      /*stack大小，单位word，也就是4字节*/
              NULL,                     /*任务参数*/
              5,                        /*任务优先�?*/
              &xHandleTaskIwdg);      /*任务句柄*/
    
    xTaskCreate(vTaskWwdg,
                "vTaskWwdg",
                512,
                NULL,
                5,
                &xHandleTaskWwdg
                );
#endif
    /*
    xTaskCreate(vTaskRtcTimeStamp,
                "vTaskRtcTimeStamp",
                512,
                NULL,
                3,
                &xHandleTaskRtcTimeStamp
                );
    */
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

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_DMA_Init();
  MX_USART6_UART_Init();
  MX_UART7_Init();
  MX_TIM17_Init();
  MX_TIM16_Init();
  MX_TIM15_Init();
  /* USER CODE BEGIN 2 */
  //__set_PRIMASK(1);
  COM_InitTypeDef ComInitStruct;
  ComInitStruct.BaudRate = 115200;
  ComInitStruct.HwFlowCtl = COM_HWCONTROL_NONE;
  ComInitStruct.Parity = COM_PARITY_NONE;
  ComInitStruct.StopBits = COM_STOPBITS_1;
  ComInitStruct.WordLength = COM_WORDLENGTH_8B;
  
  extern UART_HandleTypeDef hcom_uart[COMn];
  BSP_COM_Init(COM1, &ComInitStruct);
  HAL_UART_Transmit(hcom_uart, "stlink uart test\r\n", strlen("stlink uart test\r\n"), 0xff);
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  timer4DebugInit();
  //iwdgInit();
  //wwdgInit();
  rtcTimeStampInit();
    
  
  /*创建任务*/
  AppTaskCreate();
  
  AppObjCreate();
  
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
    BaseType_t xYieldRequired;
    UserButtonStatus ++;
    xYieldRequired = xTaskResumeFromISR(xHandleTaskUserIF);
    
    if(xYieldRequired == pdTRUE)
    {
      portYIELD_FROM_ISR(xYieldRequired);
    }
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
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30020000;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0x38000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;

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
  static uint32_t g_uiCount = 0;
  
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  BaseType_t xYieldRequired;
  UBaseType_t uxSavedInterruptStatus;
  if(htim->Instance == TIM17)
  {
    HAL_TIM_Base_Stop_IT(&htim17);
    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR(); //进入临界区
    {
      printf("HAL_TIM_PeriodElapsedCallback: enter critical area\r\n");
        printf("uxSavedInterruptStatus = %d\r\n", (int)uxSavedInterruptStatus);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus); //退出临界区
    printf("HAL_TIM_PeriodElapsedCallback: exit critical area\r\n");
    
    xYieldRequired = xTaskResumeFromISR(xHandleTaskLED);
    
    if(xYieldRequired == pdTRUE)
    {
      printf("ISR Yield required\r\n");
      portYIELD_FROM_ISR(xYieldRequired);
    }
    
  }
  
  if(htim->Instance == TIM15)
  {
    BaseType_t xResult;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    HAL_TIM_Base_Stop_IT(&htim15);
    
    g_uiCount++;
    
    xQueueSendFromISR(xQueue1, (void *)&g_uiCount, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    
    xResult = xEventGroupSetBitsFromISR(xHandleEventGroup, SET_BIT_0, &xHigherPriorityTaskWoken);
    
    if(xResult != pdFAIL)
    {
      /*消息被成功发出*/
      printf("TIM15 ISR: set event group bit 0 succeed\r\n");
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
  }
  
  if(htim->Instance == TIM16)
  {
    BaseType_t xResult;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    MSG_T *ptMsg;
    
    HAL_TIM_Base_Stop_IT(&htim16);
    
    ptMsg->ucMessageID++;
    ptMsg->ulData[1]++;
    ptMsg->usData[1]++;
    
    xQueueSendFromISR(xQueue2, (void *)&ptMsg, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    
    xResult = xEventGroupSetBitsFromISR(xHandleEventGroup, SET_BIT_1, &xHigherPriorityTaskWoken);
    
    if(xResult != pdFAIL)
    {
      /*消息被成功发出*/
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }

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
