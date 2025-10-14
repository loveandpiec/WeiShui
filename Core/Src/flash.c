/**
  * @file    flash_l4_fixed.c
  * @brief   修复版Flash操作库 for STM32L412KBT6 - 解决卡死问题
  */

#include "flash.h"
#include "stm32l4xx.h"

/**
  * @brief  等待Flash操作完成（修复版）
  */
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout)
{
    uint32_t timeout_count = 0;
    
    /* 等待操作完成 */
    while((FLASH->SR & FLASH_SR_BSY) != 0)
    {
        timeout_count++;
        if(timeout_count > Timeout)
        {
            /* 超时，强制清除BSY标志 */
            FLASH->SR &= ~FLASH_SR_BSY;
            return FLASH_TIMEOUT;
        }
    }
    
    /* 检查错误标志 */
    if(FLASH->SR & FLASH_SR_PROGERR)
    {
        FLASH->SR |= FLASH_SR_PROGERR;
        return FLASH_ERROR;
    }
    if(FLASH->SR & FLASH_SR_WRPERR)
    {
        FLASH->SR |= FLASH_SR_WRPERR;
        return FLASH_ERROR;
    }
    if(FLASH->SR & FLASH_SR_PGAERR)
    {
        FLASH->SR |= FLASH_SR_PGAERR;
        return FLASH_ERROR;
    }
    
    return FLASH_OK;
}

/**
  * @brief  解锁Flash（修复版）
  */
void FLASH_Unlock(void)
{
    /* 确保Flash不忙 */
    while(FLASH->SR & FLASH_SR_BSY);
    
    /* 清除所有可能存在的错误标志 */
    FLASH->SR |= (FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR | 
                 FLASH_SR_SIZERR | FLASH_SR_MISERR | FLASH_SR_FASTERR);
    
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        /* 写入解锁序列 */
        FLASH->KEYR = 0x45670123U;
        FLASH->KEYR = 0xCDEF89ABU;
    }
}

/**
  * @brief  擦除指定页（修复版）
  */
FLASH_Status FLASH_ErasePage(uint32_t PageAddress)
{
    FLASH_Status status;
    
    /* 确保地址有效 */
    if(PageAddress < FLASH_BASE_ADDR || PageAddress >= (FLASH_BASE_ADDR + 128*1024))
    {
        return FLASH_ERROR;
    }
    
    /* 等待Flash就绪 */
    status = FLASH_WaitForLastOperation(1000000U);
    if(status != FLASH_OK)
    {
        return status;
    }
    
    /* 设置页擦除模式 */
    FLASH->CR |= FLASH_CR_PER;
    
    /* 设置要擦除的页号 */
    uint32_t page_num = (PageAddress - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE;
    FLASH->CR &= ~FLASH_CR_PNB_Msk;
    FLASH->CR |= (page_num << FLASH_CR_PNB_Pos);
    
    /* 开始擦除 */
    FLASH->CR |= FLASH_CR_STRT;
    
    /* 等待擦除完成 */
    status = FLASH_WaitForLastOperation(1000000U);
    
    /* 清除页擦除模式 */
    FLASH->CR &= ~FLASH_CR_PER;
    
    return status;
}

/**
  * @brief  双字编程
  */
FLASH_Status FLASH_ProgramDoubleWord(uint32_t Address, uint64_t Data)
{
    FLASH_Status status;
    
    /* 等待Flash就绪 */
    status = FLASH_WaitForLastOperation(1000000U);
    if(status != FLASH_OK)
    {
        return status;
    }
    
    /* 设置编程模式 */
    FLASH->CR |= FLASH_CR_PG;
    
    /* 写入第一个字（低32位） */
    *(__IO uint32_t*)Address = (uint32_t)Data;
    
    /* 等待写入完成 */
    status = FLASH_WaitForLastOperation(1000000U);
    if(status != FLASH_OK)
    {
        FLASH->CR &= ~FLASH_CR_PG;
        return status;
    }
    
    /* 写入第二个字（高32位） */
    *(__IO uint32_t*)(Address + 4) = (uint32_t)(Data >> 32);
    
    /* 等待写入完成 */
    status = FLASH_WaitForLastOperation(1000000U);
    
    /* 清除编程模式 */
    FLASH->CR &= ~FLASH_CR_PG;
    
    return status;
}

/**
  * @brief  获取地址对应的页号
  */
uint32_t FLASH_GetPage(uint32_t Address)
{
    return ((Address - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE);
}

/**
  * @brief  锁定Flash
  */
void FLASH_Lock(void)
{
    /* 确保Flash不忙 */
    while(FLASH->SR & FLASH_SR_BSY);
    FLASH->CR |= FLASH_CR_LOCK;
}

/**
  * @brief  调试函数：打印Flash状态寄存器
  */
void FLASH_PrintStatus(void)
{
    // printf("FLASH Status Register: 0x%08lX\n", FLASH->SR);
    // printf("  BSY: %lu\n", (FLASH->SR & FLASH_SR_BSY) ? 1 : 0);
    // printf("  PROGERR: %lu\n", (FLASH->SR & FLASH_SR_PROGERR) ? 1 : 0);
    // printf("  WRPERR: %lu\n", (FLASH->SR & FLASH_SR_WRPERR) ? 1 : 0);
    // printf("  PGAERR: %lu\n", (FLASH->SR & FLASH_SR_PGAERR) ? 1 : 0);
    // printf("  SIZERR: %lu\n", (FLASH->SR & FLASH_SR_SIZERR) ? 1 : 0);
    // printf("  MISERR: %lu\n", (FLASH->SR & FLASH_SR_MISERR) ? 1 : 0);
    // printf("  FASTERR: %lu\n", (FLASH->SR & FLASH_SR_FASTERR) ? 1 : 0);
}

/**
  * @brief  简化测试函数
  */
FLASH_Status FLASH_SimpleTest(void)
{
    FLASH_Status status;
    uint32_t test_addr = 0x0801F800;  // 最后一个扇区
    
    // printf("=== Flash Test Start ===\n");
    
    // 1. 打印初始状态
    // printf("Initial Status:\n");
    FLASH_PrintStatus();
    
    // 2. 解锁Flash
    // printf("Unlocking Flash...\n");
    FLASH_Unlock();
    
    // 3. 检查解锁状态
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        // printf("ERROR: Flash unlock failed!\n");
        return FLASH_ERROR;
    }
    // printf("Flash unlocked successfully\n");
    
    // 4. 擦除页
    // printf("Erasing page at 0x%08lX...\n", test_addr);
    status = FLASH_ErasePage(test_addr);
    // printf("Erase status: %d\n", status);
    FLASH_PrintStatus();
    
    if(status != FLASH_OK)
    {
        // printf("Erase failed, locking Flash and exiting\n");
        FLASH_Lock();
        return status;
    }
    // printf("Erase successful\n");
    
    // 5. 编程测试数据
    // printf("Programming test data...\n");
    status = FLASH_ProgramDoubleWord(test_addr, 0x1122334455667788);
    // printf("Program status: %d\n", status);
    FLASH_PrintStatus();
    
    // 6. 锁定Flash
    FLASH_Lock();
    // printf("Flash locked\n");
    
    // 7. 验证数据
    uint64_t read_data = *(uint64_t*)test_addr;
    // printf("Read data: 0x%016llX\n", read_data);
    
    if(read_data == 0x1122334455667788)
    {
        // printf("=== Flash Test PASSED ===\n");
        return FLASH_OK;
    }
    else
    {
        // printf("=== Flash Test FAILED ===\n");
        return FLASH_ERROR;
    }
}

/**
  * @brief  字编程（32位）
  */
FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data)
{
    FLASH_Status status;
    
    /* 检查地址对齐 */
    if((Address & 0x3) != 0)
    {
        // printf("ERROR: Address 0x%08lX not word aligned!\n", Address);
        return FLASH_ERROR;
    }
    
    /* 等待Flash就绪 */
    status = FLASH_WaitForLastOperation(1000000);
    if(status != FLASH_OK)
    {
        // printf("ERROR: Flash not ready before programming!\n");
        return status;
    }
    
    /* 设置编程位 */
    FLASH->CR |= FLASH_CR_PG;
    
    /* 执行字编程 */
    *(__IO uint32_t*)Address = Data;
    
    /* 等待编程完成 */
    status = FLASH_WaitForLastOperation(1000000);
    
    /* 清除编程位 */
    FLASH->CR &= ~FLASH_CR_PG;
    
    if(status != FLASH_OK)
    {
        // printf("ERROR: Word programming failed at 0x%08lX\n", Address);
    }
    
    return status;
}