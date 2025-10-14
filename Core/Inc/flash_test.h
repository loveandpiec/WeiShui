/**
  * @file    flash_power_test.h
  * @brief   Flash掉电保存测试函数
  */

#ifndef __FLASH_POWER_TEST_H
#define __FLASH_POWER_TEST_H

#include "flash.h"

/* 测试数据结构 */
typedef struct {
    uint32_t magic;         // 魔术字，用于识别有效数据
    uint32_t counter;       // 计数器，每次上电递增
    uint32_t timestamp;     // 时间戳
    uint8_t  data[16];      // 测试数据
    uint32_t crc;           // CRC校验
} FlashTestData_t;

/* 测试地址 */
#define TEST_DATA_ADDR     FLASH_LAST_SECTOR_ADDR

/* 魔术字 */
#define FLASH_MAGIC_WORD   0x55AA1234

/* 函数声明 */
void FLASH_PowerTest_Init(void);
void FLASH_PowerTest_Run(void);
uint32_t FLASH_CalculateCRC(uint8_t *data, uint32_t length);
void FLASH_PowerTest_DisplayInfo(void);
void FLASH_PowerTest_Complete(void);

#endif /* __FLASH_POWER_TEST_H */