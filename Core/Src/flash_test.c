/**
  * @file    flash_power_test.c
  * @brief   Flash掉电保存测试函数实现 - 修正版
  */

#include "flash_test.h"
#include <stdio.h>
#include <string.h>

/**
  * @brief  简单的CRC32计算
  */
uint32_t FLASH_CalculateCRC(uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    
    for(uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for(uint32_t j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc = crc >> 1;
        }
    }
    
    return ~crc;
}

/**
  * @brief  检查是否需要擦除
  */
FLASH_Status FLASH_CheckAndEraseIfNeeded(uint32_t address, FlashTestData_t *new_data)
{
    FlashTestData_t *existing_data = (FlashTestData_t*)address;
    
    // 如果魔术字不匹配，说明需要擦除并重新初始化
    if(existing_data->magic != FLASH_MAGIC_WORD)
    {
        // printf("Magic word mismatch, erasing page...\n");
        return FLASH_ErasePage(address);
    }
    
    // 如果只是更新计数器，检查对应位置是否可编程
    // 这里简化处理：每次都擦除整个页
    // printf("Existing data found, erasing page for update...\n");
    return FLASH_ErasePage(address);
}

/**
  * @brief  验证Flash区域内容
  */
void FLASH_VerifyArea(uint32_t Address, uint32_t length)
{
    // printf("Verifying Flash at 0x%08lX:\n", Address);
    uint8_t *addr = (uint8_t*)Address;
    uint32_t errors = 0;
    
    for(uint32_t i = 0; i < length; i += 4)
    {
        uint32_t data = *(uint32_t*)(addr + i);
        if(data != 0xFFFFFFFF)
        {
            // printf("  0x%08lX: 0x%08lX\n", (uint32_t)(addr + i), data);
            errors++;
        }
    }
    
    if(errors == 0)
    {
        // printf("  All addresses contain 0xFFFFFFFF (erased)\n");
    }
    else
    {
        // printf("  Found %lu non-0xFFFFFFFF values\n", errors);
    }
}

/**
  * @brief  初始化掉电保存测试
  */
void FLASH_PowerTest_Init(void)
{
    FlashTestData_t *test_data = (FlashTestData_t*)TEST_DATA_ADDR;
    
    // printf("\n=== Flash Power Loss Test ===\n");
    // printf("Test address: 0x%08lX\n", TEST_DATA_ADDR);
    
    // 检查是否已有有效数据
    if(test_data->magic == FLASH_MAGIC_WORD)
    {
        // printf("Found existing test data:\n");
        FLASH_PowerTest_DisplayInfo();
        
        // 验证CRC
        uint32_t calculated_crc = FLASH_CalculateCRC((uint8_t*)test_data, 
                                                   sizeof(FlashTestData_t) - sizeof(uint32_t));
        
        if(calculated_crc == test_data->crc)
        {
            // printf("CRC Check: PASS\n");
        }
        else
        {
            // printf("CRC Check: FAIL (Stored: 0x%08lX, Calculated: 0x%08lX)\n", 
            //        test_data->crc, calculated_crc);
        }
    }
    else
    {
        // printf("No valid test data found (Magic: 0x%08lX)\n", test_data->magic);
        // printf("Flash content at test address:\n");
        FLASH_VerifyArea(TEST_DATA_ADDR, 32);
    }
}

/**
  * @brief  显示测试信息
  */
void FLASH_PowerTest_DisplayInfo(void)
{
    FlashTestData_t *test_data = (FlashTestData_t*)TEST_DATA_ADDR;
    
    // printf("Magic: 0x%08lX\n", test_data->magic);
    // printf("Boot Counter: %lu\n", test_data->counter);
    // printf("Timestamp: %lu\n", test_data->timestamp);
    // printf("Test Data: ");
    for(int i = 0; i < 16; i++)
    {
        printf("%02X ", test_data->data[i]);
    }
    printf("\nCRC: 0x%08lX\n", test_data->crc);
}

/**
  * @brief  运行掉电保存测试
  */
void FLASH_PowerTest_Run(void)
{
    FLASH_Status status;
    FlashTestData_t new_data;
    FlashTestData_t *existing_data = (FlashTestData_t*)TEST_DATA_ADDR;
    
    printf("\n--- Running Power Loss Test ---\n");
    
    // 准备新数据
    new_data.magic = FLASH_MAGIC_WORD;
    
    // 如果已有数据，计数器递增，否则从1开始
    if(existing_data->magic == FLASH_MAGIC_WORD)
    {
        new_data.counter = existing_data->counter + 1;
        printf("Incrementing boot counter: %lu -> %lu\n", 
               existing_data->counter, new_data.counter);
    }
    else
    {
        new_data.counter = 1;
        printf("First boot, counter initialized to: %lu\n", new_data.counter);
    }
    
    // 设置时间戳
    new_data.timestamp = 23454235345;
    
    // 生成测试数据模式
    for(int i = 0; i < 16; i++)
    {
        new_data.data[i] = (i << 4) | (new_data.counter & 0x0F);
    }
    
    // 计算CRC（不包括CRC字段本身）
    new_data.crc = FLASH_CalculateCRC((uint8_t*)&new_data, sizeof(FlashTestData_t) - sizeof(uint32_t));
    
    printf("New data prepared:\n");
    printf("  Counter: %lu\n", new_data.counter);
    printf("  Timestamp: %lu\n", new_data.timestamp);
    printf("  CRC: 0x%08lX\n", new_data.crc);
    
    // 解锁Flash
    printf("\nUnlocking Flash...\n");
    FLASH_Unlock();
    
    // 总是擦除整个页（简化处理）
    printf("Erasing Flash page...\n");
    status = FLASH_ErasePage(TEST_DATA_ADDR);
    if(status != FLASH_OK)
    {
        printf("ERROR: Erase failed with status: %d\n", status);
        FLASH_PrintStatus();
        FLASH_Lock();
        return;
    }
    printf("Erase successful\n");
    
    // 验证擦除结果
    printf("Verifying erase...\n");
    uint32_t *check_addr = (uint32_t*)TEST_DATA_ADDR;
    for(int i = 0; i < 8; i++)
    {
        if(check_addr[i] != 0xFFFFFFFF)
        {
            printf("WARNING: Address 0x%08lX = 0x%08lX (expected 0xFFFFFFFF)\n", 
                   (uint32_t)&check_addr[i], check_addr[i]);
        }
    }
    
    // 编程新数据
    printf("Programming new data...\n");
    
    // 使用双字编程提高效率（一次编程8字节）
    uint32_t program_address = TEST_DATA_ADDR;
    
    // 编程前32字节（magic + counter + timestamp + data[0-12]）
    for(int i = 0; i < 4; i++)
    {
        uint64_t double_word = *(uint64_t*)((uint8_t*)&new_data + i * 8);
        status = FLASH_ProgramDoubleWord(program_address + i * 8, double_word);
        if(status != FLASH_OK)
        {
            printf("ERROR: Double word programming failed at 0x%08lX, status: %d\n", 
                   program_address + i * 8, status);
            FLASH_PrintStatus();
            FLASH_Lock();
            return;
        }
    }
    
    // 编程最后8字节（data[12-15] + crc）
    uint64_t last_double_word = *(uint64_t*)((uint8_t*)&new_data + 32);
    status = FLASH_ProgramDoubleWord(program_address + 32, last_double_word);
    if(status != FLASH_OK)
    {
        printf("ERROR: Last double word programming failed, status: %d\n", status);
        FLASH_PrintStatus();
        FLASH_Lock();
        return;
    }
    
    printf("All data programmed successfully\n");
    
    // 锁定Flash
    FLASH_Lock();
    printf("Flash locked\n");
    
    // 验证写入的数据
    printf("\nVerifying written data...\n");
    FlashTestData_t *verify_data = (FlashTestData_t*)TEST_DATA_ADDR;
    
    uint32_t calculated_crc = FLASH_CalculateCRC((uint8_t*)verify_data, 
                                               sizeof(FlashTestData_t) - sizeof(uint32_t));
    
    printf("Verification Results:\n");
    printf("  Magic: 0x%08lX %s\n", verify_data->magic, 
           verify_data->magic == FLASH_MAGIC_WORD ? "OK" : "FAIL");
    printf("  Counter: %lu %s\n", verify_data->counter,
           verify_data->counter == new_data.counter ? "OK" : "FAIL");
    printf("  Timestamp: %lu %s\n", verify_data->timestamp,
           verify_data->timestamp == new_data.timestamp ? "OK" : "FAIL");
    printf("  CRC: 0x%08lX %s\n", verify_data->crc,
           verify_data->crc == calculated_crc ? "OK" : "FAIL");
    
    if(verify_data->magic == FLASH_MAGIC_WORD &&
       verify_data->counter == new_data.counter &&
       verify_data->timestamp == new_data.timestamp &&
       verify_data->crc == calculated_crc)
    {
        printf("\n=== POWER LOSS TEST: SUCCESS ===\n");
        printf("Boot count updated to: %lu\n", new_data.counter);
        printf("Now you can:\n");
        printf("1. Power off the board\n");
        printf("2. Wait a few seconds\n");
        printf("3. Power on again\n");
        printf("4. Check if the counter increments correctly\n");
    }
    else
    {
        printf("\n=== POWER LOSS TEST: FAILED ===\n");
        printf("Data verification failed!\n");
    }
}

/**
  * @brief  完整的掉电测试流程
  */
void FLASH_PowerTest_Complete(void)
{
    printf("\n"
           "=========================================\n"
           "    FLASH POWER LOSS RECOVERY TEST\n"
           "=========================================\n");
    
    // 初始化并显示当前状态
    FLASH_PowerTest_Init();
    
    // 运行测试（更新数据）
    FLASH_PowerTest_Run();
    
    printf("\n=========================================\n");
}