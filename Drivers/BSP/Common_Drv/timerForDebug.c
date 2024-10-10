#include "timerForDebug.h"
#include <stdint.h>
#include <stdio.h>
#include "stm32h7xx.h"
#include "main.h"

/*定时器频率，50us一次中断*/
#define timerInterruptFrequency         20000

/*中断优先级*/
#define timerHighestPriority            1

/*被系统调用*/
volatile uint32_t ulHighFrequencyTimerTicks = 0UL;
TIM_HandleTypeDef    TimHandle;
/* Prescaler declaration */
static uint32_t uwPrescalerValue = 0;

void timer4DebugInit(void)
{
    uwPrescalerValue = (uint32_t)(SystemCoreClock / (2*timerInterruptFrequency)) - 1;
    
    
    TimHandle.Instance = TIM3;
    TimHandle.Init.Period            = 1;
    TimHandle.Init.Prescaler         = uwPrescalerValue;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;
    
    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
      printf("time base init error\n");
    }
    
    
    if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
    {
      printf("time base start it error\n");
    }
        
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /*##-1- Enable peripheral clock #################################*/
  /* TIMx Peripheral clock enable */
  __HAL_RCC_TIM3_CLK_ENABLE();
  
  /*##-2- Configure the NVIC for TIMx ########################################*/
  /* Set the TIMx priority */
  HAL_NVIC_SetPriority(TIM3_IRQn, timerHighestPriority, 0);

  /* Enable the TIMx global Interrupt */
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM3_IRQHandler(void)
{
  ulHighFrequencyTimerTicks++;
  HAL_TIM_IRQHandler(&TimHandle);
}

