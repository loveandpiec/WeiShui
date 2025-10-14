/**
  * @file    flash_l4.h
  * @brief   Flash操作库 for STM32L412KBT6 - LL库版本
  */

#ifndef __FLASH_L4_H
#define __FLASH_L4_H

#include "stm32l412xx.h"

/* Flash操作状态定义 */
typedef enum {
    FLASH_OK       = 0x00U,
    FLASH_ERROR    = 0x01U,
    FLASH_BUSY     = 0x02U,
    FLASH_TIMEOUT  = 0x03U
} FLASH_Status;

/* Flash页大小 */
#define FLASH_PAGE_SIZE           2048U  /* 2KB */

/* Flash起始地址 */
#define FLASH_BASE_ADDR           ((uint32_t)0x08000000)

/* STM32L412KBT6最后一个扇区地址 */
#define FLASH_LAST_SECTOR_ADDR    0x0801F800

/* 函数声明 */
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout);
void FLASH_Unlock(void);
void FLASH_Lock(void);
FLASH_Status FLASH_ErasePage(uint32_t PageAddress);
FLASH_Status FLASH_ProgramDoubleWord(uint32_t Address, uint64_t Data);
FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);
uint32_t FLASH_GetPage(uint32_t Address);
FLASH_Status FLASH_SimpleTest(void);

#endif /* __FLASH_L4_H */