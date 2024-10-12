#include "cpu_flash.h"
#include "stm32h7xx.h"

#if 0
uint32_t getSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if(((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) || \
      ((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2)))
  {
    sector = FLASH_SECTOR_0;
  }
}

uint8_t eraseCpuFlash(uint32_t ulFlashAddr)
{
  uint32_t FirstSector = 0, NbrOfSectors = 0;
  FLASH_EraseInitTypeDef        EraseInitStruct;
  uint32_t sectorError = 0;
  uint8_t re;
  
  /*解锁FLASH*/
  HAL_FLASH_Unlock();
  
  /*获取Flash地址所在的扇区*/
  FirstSector = getSector(ulFlashAddr);
  
  
}
#endif
