#include "cpu_flash.h"
#include "stm32h7xx.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>


uint32_t getSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if(((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1)) || \
      ((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2)))
  {
    sector = FLASH_SECTOR_0;
  }
  else if (((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1)) || \
	  ((Address < ADDR_FLASH_SECTOR_2_BANK2) && (Address >= ADDR_FLASH_SECTOR_1_BANK2)))    
  {
          sector = FLASH_SECTOR_1;  
  }
  else if (((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1)) || \
    ((Address < ADDR_FLASH_SECTOR_3_BANK2) && (Address >= ADDR_FLASH_SECTOR_2_BANK2)))    
  {
          sector = FLASH_SECTOR_2;  
  }
  else if (((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1)) || \
    ((Address < ADDR_FLASH_SECTOR_4_BANK2) && (Address >= ADDR_FLASH_SECTOR_3_BANK2)))    
  {
          sector = FLASH_SECTOR_3;  
  }
  else if (((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1)) || \
    ((Address < ADDR_FLASH_SECTOR_5_BANK2) && (Address >= ADDR_FLASH_SECTOR_4_BANK2)))    
  {
          sector = FLASH_SECTOR_4;  
  }
  else if (((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1)) || \
    ((Address < ADDR_FLASH_SECTOR_6_BANK2) && (Address >= ADDR_FLASH_SECTOR_5_BANK2)))    
  {
          sector = FLASH_SECTOR_5;  
  }
  else if (((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1)) || \
    ((Address < ADDR_FLASH_SECTOR_7_BANK2) && (Address >= ADDR_FLASH_SECTOR_6_BANK2)))    
  {
          sector = FLASH_SECTOR_6;  
  }
  else if (((Address < ADDR_FLASH_SECTOR_0_BANK2) && (Address >= ADDR_FLASH_SECTOR_7_BANK1)) || \
    ((Address < CPU_FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7_BANK2)))
  {
          sector = FLASH_SECTOR_7;  
  }
  else
  {
          sector = FLASH_SECTOR_7;
  }

  return sector;
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
  
  /*固定擦除一个扇区*/
  NbrOfSectors = 1;
  
  /*擦除扇区配置*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  
  if(ulFlashAddr >= ADDR_FLASH_SECTOR_0_BANK2)
  {
    EraseInitStruct.Banks = FLASH_BANK_2;
  }
  else
  {
    EraseInitStruct.Banks = FLASH_BANK_1;
  }
  
  EraseInitStruct.Sector = FirstSector;
  EraseInitStruct.NbSectors = NbrOfSectors;
  
  re = HAL_FLASHEx_Erase(&EraseInitStruct,   &sectorError);
  
  HAL_FLASH_Lock();
  
  return re;
  
}

uint8_t cmpCpuFlash(uint32_t ulFlashAddr, uint8_t *buffer, uint32_t size)
{
  uint32_t i;
  uint8_t equFlag;
  uint8_t ucByte;
  
  if(ulFlashAddr + size > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
  {
    return FLASH_PARAM_ERR;
  }
  
  if(size == 0)
  {
    return FLASH_IS_EQU;
  }
  
  equFlag = 1; //先假设写入的数据和存储数据都相等
  
  for(i = 0; i < size; i++)
  {
    ucByte = *(uint8_t *)ulFlashAddr;
    
    if(ucByte != *buffer)
    {
      if(ucByte != 0xFF)
      {
        return FLASH_REQ_ERASE;
      }
      else
      {
        equFlag = 0; //不相等，需要写入
      }
    }
    ulFlashAddr++;
    buffer++;
  }
  
  if(equFlag == 1)
  {
    return FLASH_IS_EQU;
  }
  else
  {
    return FLASH_REQ_WRITE;
  }
  
}

uint8_t writeCpuFlash(uint32_t ulFlashAddr, uint8_t *buffer, uint32_t size)
{
  uint32_t i;
  uint8_t ret;
  
  /*偏移地址超过芯片容量，不改写输出缓冲区*/
  if(ulFlashAddr + size > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
  {
    return 1;
  }
  
  /*长度为0时不继续操作*/
  if(size == 0)
  {
    return 0;
  }
  
  ret = cmpCpuFlash(ulFlashAddr, buffer, size);
  
  if(ret == FLASH_IS_EQU)
  {
    return 0;
  }
  
  __set_PRIMASK(1);             //关闭中断
  
  HAL_FLASH_Unlock();
  
  for(i = 0; i < size / 32; i++)
  {
    uint64_t FlashWord[4];
    
    memcpy((char *)FlashWord, buffer, 32);
    
    buffer += 32;
    
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, (uint32_t)ulFlashAddr, (uint32_t)((uint64_t)FlashWord)) == HAL_OK)
    {
      ulFlashAddr += 32;
    }
    else
    {
      printf("flash program error\r\n");
      goto err;
    }
  }
  
  /*长度不是32字节的整数倍*/
  if(size % 32)
  {
    uint64_t FlashWord[4];
    
    FlashWord[0] = 0;
    FlashWord[1] = 0;
    FlashWord[2] = 0;
    FlashWord[3] = 0;
    memcpy((char *)FlashWord, buffer, size % 32);
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, (uint32_t)ulFlashAddr, (uint32_t)((uint64_t)FlashWord)) == HAL_OK)
    {
      
    }
    else
    {
      goto err;
    }
  }
  
  /*Flash加锁*/
  HAL_FLASH_Lock();
  
  __set_PRIMASK(0);             //开中断
  
  printf("write cpu flash success\r\n");
  
  return 0;
  
err:
  /*Flash加锁*/
  HAL_FLASH_Lock();
  
  __set_PRIMASK(0);
  
  printf("write cpu flash error\r\n");
  
  return 1;
  
}

uint8_t readCpuFlash(uint32_t ulFlashAddr,  uint8_t *readBuffer, uint32_t size)
{
  uint32_t i;
  
  if(ulFlashAddr + size > CPU_FLASH_BASE_ADDR + CPU_FLASH_SIZE)
  {
    printf("read size out of boundary\r\n");
    return 1;
  }
  
  /*长度为0时不继续操作，否则起始地址为奇地址会出错*/
  if(size == 0)
  {
    return 1;
  }
  
  for(i = 0; i < size; i++)
  {
    *readBuffer = *(uint8_t *)ulFlashAddr;
    readBuffer ++;
    ulFlashAddr++;
  }
  
  return 0;
}


