#include "uart_dma.h"
#include "iwdg.h"
#include <stdio.h>

IWDG_HandleTypeDef IwdgHandle;

void iwdgInit(void)
{
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST) != RESET)
  {
    /* IWDGRST flag set: Turn LED1 on */
    printf("the system has resumed from IWDG reset\r\n");
    /* Clear reset flags anyway */
    __HAL_RCC_CLEAR_RESET_FLAGS();
    printf("System reset flag clear\r\n");
  }
  
  IwdgHandle.Instance = IWDG1;
  IwdgHandle.Init.Prescaler = IWDG_PRESCALER_16;
  IwdgHandle.Init.Reload = (32000 * 762)  / (16 * 1000); /*762ms*/
  IwdgHandle.Init.Window = (32000 * 400) / (16 * 1000); /*400ms*/
  
  if(HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
  {
    printf("HAL_IWDG_Init failed\r\n");
  }
  
  

}
