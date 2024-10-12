#include "wwdg.h"
#include "stm32h7xx.h"
#include <stdio.h>

WWDG_HandleTypeDef   WwdgHandle;
uint32_t wwdgDelayNum;
/**
  * @brief  Timeout calculation function.
  *         This function calculates any timeout related to 
  *         WWDG with given prescaler and system clock.
  * @param  timevalue: period in term of WWDG counter cycle.
  * @retval None
  */
static uint32_t TimeoutCalculation(uint32_t timevalue)
{
  uint32_t timeoutvalue = 0;
  uint32_t pclk1 = 0;
  uint32_t wdgtb = 0;

  /* Get PCLK1 value */
  pclk1 = HAL_RCC_GetPCLK1Freq();

  /* get prescaler */
  switch(WwdgHandle.Init.Prescaler)
  {
    case WWDG_PRESCALER_1:   wdgtb = 1;   break;
    case WWDG_PRESCALER_2:   wdgtb = 2;   break;
    case WWDG_PRESCALER_4:   wdgtb = 4;   break;
    case WWDG_PRESCALER_8:   wdgtb = 8;   break;
    case WWDG_PRESCALER_16:  wdgtb = 16;  break;
    case WWDG_PRESCALER_32:  wdgtb = 32;  break;
    case WWDG_PRESCALER_64:  wdgtb = 64;  break;
    case WWDG_PRESCALER_128: wdgtb = 128; break;

    default: printf("wwdg prescaler setted wrong\r\n"); break;
  }
  
  
  /* calculate timeout */
  timeoutvalue = ((4096 * wdgtb * timevalue) / (pclk1 / 1000));

  return timeoutvalue;
}


void wwdgInit(void)
{
  /*##-1- Check if the system has resumed from WWDG reset ####################*/
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST) != RESET)
  {
    printf("system has resumed from WWDG reset\r\n");
  }
  __HAL_RCC_CLEAR_RESET_FLAGS();
  
  /* Enable system wide reset */
  HAL_RCCEx_WWDGxSysResetConfig(RCC_WWDG1);
  
  WwdgHandle.Instance = WWDG1;
  WwdgHandle.Init.Prescaler = WWDG_PRESCALER_128;
  WwdgHandle.Init.Window    = 0x50;
  WwdgHandle.Init.Counter   = 0x7F;
  WwdgHandle.Init.EWIMode   = WWDG_EWI_DISABLE;
  
  if (HAL_WWDG_Init(&WwdgHandle) != HAL_OK)
  {
    /* Initialization Error */
    printf("wwdg init failed\r\n");
  }
  
  wwdgDelayNum = TimeoutCalculation((WwdgHandle.Init.Counter - WwdgHandle.Init.Window) + 1) + 1;
  
}

/**
  * @brief WWDG MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param hwwdg: WWDG handle pointer
  * @retval None
  */
void HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg)
{
  /* WWDG Peripheral clock enable */
  __HAL_RCC_WWDG1_CLK_ENABLE();
}