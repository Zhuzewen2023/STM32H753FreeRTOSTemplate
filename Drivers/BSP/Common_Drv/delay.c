#include "delay.h"
#include "stm32h7xx.h"
#include <stdint.h>

void delay_us(uint32_t nus)
{
  uint32_t ticks;
  uint32_t told, tnow, reload, tcnt = 0;
  
  reload = SysTick->LOAD;
  ticks = nus * (SystemCoreClock / 1000000);
  told = SysTick->VAL;
  
  while(1)
  {
    tnow = SysTick->VAL;
    if(tnow != told)
    {
      if(tnow < told)
      {
        tcnt += told - tnow;
      }
      else
      {
        tcnt += told + reload - tnow;
      }
      told = tnow;
      if(tcnt >= ticks)
      {
        break;
      }
    }
  }
}