#include "bootLoader.h"
#include "stm32h7xx.h"
#include <stdint.h>
#include <stdio.h>

void JumpToBootLoader(void)
{
  uint32_t i = 0;
  void (*SysMemBootJump)(void);
  volatile uint32_t BootAddr = 0x1FF09800;
  
  DISABLE_INT();
  
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  
  /*设置所有时钟到默认状态，使用HSI时钟*/
  HAL_RCC_DeInit();
  
  /*关闭所有中断，清除所有中断挂起标志*/
  for(i = 0; i < 8; i++)
  {
    NVIC->ICER[i] = 0xFFFFFFFF;
    NVIC->ICPR[i] = 0xFFFFFFFF;
    
  }
  
  /*使能全局中断*/
  ENABLE_INT();
  
  /* 跳转到系统BootLoader，首地址存放的是给MSP用的堆栈栈顶指针，地址+4是给PC程序计数器用的复位中断服务程序地址 */
  SysMemBootJump = (void (*)(void))(*(uint32_t *)(BootAddr + 4));
  
  /*设置主堆栈指针*/
  __set_MSP(*(uint32_t *)BootAddr);
  
  /*RTOS工程，设置为特权级模式，使用MSP指针*/
  __set_CONTROL(0);
  
  /*跳转到系统BootLoader*/
  SysMemBootJump();
  
  /*跳转成功的话，不会执行到这里*/
  printf("jump to bootloader failed\r\n");
  
}