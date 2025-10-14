#include "rtc.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_rtc.h"
#include "flash.h"  // 包含Flash操作库
#include <stdio.h>
// 定义Flash存储地址（使用最后一个扇区）
#define RTC_CONFIG_ADDR            FLASH_LAST_SECTOR_ADDR
#define HISTORY_INIT_FLAG_OFFSET   0x00
#define ALARM_INTERVAL_OFFSET      0x04
#define HISTORY_INIT_FLAG          0x12345678
#define DEFAULT_DEVICE_ID          0x01
/**
  * @brief  从Flash读取RTC配置
  */
History_Config_T* get_history_config(void)
{
    return (History_Config_T*)RTC_CONFIG_ADDR;
}

/**
  * @brief  存储完整RTC配置到Flash
  */
void store_history_config(const History_Config_T* config)
{
    FLASH_Status status;
    
    // 这里config应该指向有效的RAM数据，而不是Flash地址
    uint8_t* config_bytes = (uint8_t*)config;
    uint32_t config_size = sizeof(History_Config_T);
    // 解锁Flash
    FLASH_Unlock();
    
    // 擦除整个配置页
    status = FLASH_ErasePage(RTC_CONFIG_ADDR);
    if(status != FLASH_OK)
    {
        // printf("ERROR: Flash erase failed: %d\n", status);
        FLASH_Lock();
        return;
    }
    // printf("Flash erase successful\n");
    
    for (uint32_t i = 0; i < config_size; i += 8) 
    {
        uint64_t data = 0;
        
        // 将8个字节组合成一个uint64_t
        for (uint32_t j = 0; j < 8 && (i + j) < config_size; j++)
            data |= ((uint64_t)config_bytes[i + j] << (j * 8));
        
        status = FLASH_ProgramDoubleWord(RTC_CONFIG_ADDR + i, data);
        if (status != 0) {
            break;
        }
    }

    if(status != FLASH_OK)
    {
        // printf("ERROR: Flash programming failed: %d\n", status);
        FLASH_Lock();
        return;
    }
    
    // 锁定Flash
    FLASH_Lock();
    // printf("Configuration stored successfully\n");
}

/**
  * @brief  检查RTC是否已经初始化
  */
uint8_t is_rtc_initialized(void)
{
    History_Config_T *config = get_history_config();
    return (config->init_flag == HISTORY_INIT_FLAG);
}

/**
  * @brief  等待RTC同步
  * @retval RTC_ERROR_NONE 成功，其他值失败
  */
uint32_t WaitForSynchro_RTC(void)
{
    /* 清除RSF标志 */
    LL_RTC_ClearFlag_RS(RTC);
    
    uint32_t timeout = 100000; // 约100ms
    
    /* 等待寄存器同步 */
    while (LL_RTC_IsActiveFlag_RS(RTC) != 1) {
        if (timeout == 0) {
            return 1;
        }
        timeout--;
    }
    
    return 0;
}

/**
  * @brief  更新闹钟时间（使用Flash存储的间隔）
  */
void update_alarm_time(void)
{   
    // 等待RTC同步
    WaitForSynchro_RTC();
    
    // 获取当前时间（BCD格式）
    uint8_t current_hours = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
    uint8_t current_minutes = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
    uint8_t current_seconds = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
    
    // printf("Current RTC Time: %02d:%02d:%02d\n", current_hours, current_minutes, current_seconds);
    
    // 从Flash获取闹钟间隔
    uint32_t alarm_interval = get_history_config()->alarm_interval;
    // printf("Alarm interval: %lu seconds\n", alarm_interval);
    
    // 计算闹钟时间
    uint32_t current_total_seconds = current_hours * 3600U + current_minutes * 60U + current_seconds;
    uint32_t alarm_total_seconds = current_total_seconds + alarm_interval;
    
    if (alarm_total_seconds >= 86400U) {
        alarm_total_seconds -= 86400U;
        // printf("Alarm crosses midnight\n");
    }
    
    uint8_t alarm_hours = alarm_total_seconds / 3600U;
    uint8_t alarm_minutes = (alarm_total_seconds % 3600U) / 60U;
    uint8_t alarm_seconds = alarm_total_seconds % 60U;
    
    // printf("Calculated alarm time: %02d:%02d:%02d\n", alarm_hours, alarm_minutes, alarm_seconds);
    
    // 设置闹钟
    Set_RTC_Alarm_LL(alarm_hours, alarm_minutes, alarm_seconds);
    
    // printf("=== Alarm Update Complete ===\n\n");
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
void MX_RTC_Init(void)
{
    /* USER CODE BEGIN RTC_Init 0 */
    /* USER CODE END RTC_Init 0 */
    LL_RTC_InitTypeDef RTC_InitStruct = {0};
    LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
    LL_RTC_DateTypeDef RTC_DateStruct = {0};

    if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
    {
        FlagStatus pwrclkchanged = RESET;
        /* Update LSE configuration in Backup Domain control register */
        /* Requires to enable write access to Backup Domain if necessary */
        if (LL_APB1_GRP1_IsEnabledClock (LL_APB1_GRP1_PERIPH_PWR) != 1U)
        {
            /* Enables the PWR Clock and Enables access to the backup domain */
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
            pwrclkchanged = SET;
        }
        if (LL_PWR_IsEnabledBkUpAccess () != 1U)
        {
            /* Enable write access to Backup domain */
            LL_PWR_EnableBkUpAccess();
            while (LL_PWR_IsEnabledBkUpAccess () == 0U)
            {
            }
        }
        LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
        LL_RCC_LSE_Enable();

        /* Wait till LSE is ready */
        while(LL_RCC_LSE_IsReady() != 1)
        {
        }
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
        /* Restore clock configuration if changed */
        if (pwrclkchanged == SET)
        {
            LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_PWR);
        }
    }

    /* Peripheral clock enable */
    LL_RCC_EnableRTC();

    /* RTC interrupt Init */
    // 1. 配置EXTI线17 (RTC Alarm)
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_18;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_EVENT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    NVIC_SetPriority(RTC_Alarm_IRQn, 0x0F);
    NVIC_EnableIRQ(RTC_Alarm_IRQn);
    
    /* USER CODE BEGIN RTC_Init 1 */
    // 显示当前Flash配置
    // show_rtc_config();
    
    // 检查RTC是否已经初始化过
    if (!is_rtc_initialized()) 
    {
        // printf("RTC first time initialization...\n");
        
        // 第一次初始化：需要完整配置
        RTC_DateStruct.WeekDay = LL_RTC_WEEKDAY_MONDAY;
        RTC_DateStruct.Month = LL_RTC_MONTH_JANUARY;
        RTC_DateStruct.Day = 0x1;
        RTC_DateStruct.Year = 0;
        LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_DateStruct);
        
        // 设置默认时间
        RTC_Set_Time(0, 0, 0);
        
        // 存储默认闹钟间隔到Flash
        // store_alarm_interval(60);  // 默认60秒
        History_Config_T config = {.init_flag = HISTORY_INIT_FLAG,
                                  .alarm_interval = 60,
                                  .device_id = DEFAULT_DEVICE_ID};
        store_history_config(&config);
        
        // printf("RTC initialized with default settings\n");
    }
    // 更新闹钟时间
    update_alarm_time();
    /* USER CODE END RTC_Init 2 */
}


/**
  * @brief  进入RTC初始化模式 - 官方方式
  * @retval RTC_ERROR_NONE 成功，其他值失败
  */
uint32_t Enter_RTC_InitMode(void)
{
    /* 设置初始化模式 */
    LL_RTC_EnableInitMode(RTC);
    
    uint32_t timeout = 100000; // 约100ms
    
    /* 检查初始化模式是否设置成功 */
    while (LL_RTC_IsActiveFlag_INIT(RTC) != 1) {
        if (timeout == 0) {
            return 1;
        }
        timeout--;
    }
    
    return 0;
}

/**
  * @brief  退出RTC初始化模式 - 官方方式
  * @retval RTC_ERROR_NONE 成功，其他值失败
  */
uint32_t Exit_RTC_InitMode(void)
{
    LL_RTC_DisableInitMode(RTC);
    
    /* 等待同步 */
    return WaitForSynchro_RTC();
}

// 设置RTC闹钟
void Set_RTC_Alarm_LL(uint8_t h, uint8_t m, uint8_t s)
{
//    printf("\n>>> Setting RTC Alarm to %02d:%02d:%02d\n", h, m, s);
    
    // 1. 禁用闹钟A
    LL_RTC_ALMA_Disable(RTC);
    
    // 2. 清除闹钟标志
    LL_RTC_ClearFlag_ALRA(RTC);
    
    // 3. 解锁写保护
    LL_RTC_DisableWriteProtection(RTC);
    
    // 4. 进入初始化模式
    if (!Enter_RTC_InitMode()) {
        LL_RTC_EnableWriteProtection(RTC);
    }
    
    // 5. 配置闹钟时间 - 使用官方方式
    // 注意：官方示例使用BCD格式，所以需要转换
    LL_RTC_ALMA_ConfigTime(RTC, LL_RTC_ALMA_TIME_FORMAT_PM, 
                          __LL_RTC_CONVERT_BIN2BCD(h),
                          __LL_RTC_CONVERT_BIN2BCD(m), 
                          __LL_RTC_CONVERT_BIN2BCD(s));
    
    // 6. 设置闹钟掩码 - 忽略日期，只匹配时间
    LL_RTC_ALMA_SetMask(RTC, LL_RTC_ALMA_MASK_DATEWEEKDAY);
    
    // 7. 使能闹钟
    LL_RTC_ALMA_Enable(RTC);
    
    // 8. 使能闹钟中断
    LL_RTC_EnableIT_ALRA(RTC);
    
    // 9. 退出初始化模式
    if (!Exit_RTC_InitMode()) 
    {
        // printf("ERROR: Cannot exit INIT mode!\n");
        LL_RTC_EnableWriteProtection(RTC);
    }
    
    // 10. 重新使能写保护
    LL_RTC_EnableWriteProtection(RTC);
    
    // printf(">>> Alarm set SUCCESS\n");

}

// 读取RTC时间
uint32_t RTC_Get_Time(LL_RTC_TimeTypeDef *time)
{   
    return LL_RTC_TIME_Get(RTC);
}

// 设置RTC时间
void RTC_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    LL_RTC_TimeTypeDef time_init = {0};
    
    // 使能后备电源
    if (LL_PWR_IsEnabledBkUpAccess () != 1U)
    {
        /* Enable write access to Backup domain */
        LL_PWR_EnableBkUpAccess();
        while (LL_PWR_IsEnabledBkUpAccess () == 0U);
    }
    
    // 进入初始化模式
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_EnterInitMode(RTC);
    
    // 填充时间结构体
    time_init.Hours = hours;
    time_init.Minutes = minutes;
    time_init.Seconds = seconds;
    time_init.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;
    
    // 使用LL库函数设置时间
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &time_init);
    
    // 退出初始化模式
    LL_RTC_ExitInitMode(RTC);
    LL_RTC_EnableWriteProtection(RTC);
    LL_PWR_DisableBkUpAccess();

    // printf("Time set to: %02d:%02d:%02d\n", hours, minutes, seconds);
}

// 设置RTC日期
void RTC_Set_Date(uint8_t year, uint8_t month, uint8_t day)
{
    LL_RTC_DateTypeDef date_init = {0};
    
    // 进入初始化模式
    LL_RTC_DisableWriteProtection(RTC);
    LL_RTC_EnterInitMode(RTC);
    
    // 填充日期结构体
    date_init.Year = year;
    date_init.Month = month;
    date_init.Day = day;
    date_init.WeekDay = LL_RTC_WEEKDAY_MONDAY;
    
    // 使用LL库函数设置日期
    LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &date_init);
    
    // 退出初始化模式
    LL_RTC_ExitInitMode(RTC);
    LL_RTC_EnableWriteProtection(RTC);
}

// RTC闹钟中断处理函数
void RTC_Alarm_IRQHandler(void)
{
    if(LL_RTC_IsActiveFlag_ALRA(RTC) != RESET)
    {
        // 清除闹钟标志位
        LL_RTC_ClearFlag_ALRA(RTC);
        
        // 处理闹钟事件
        // printf("RTC Alarm Triggered!\n");
        
        // 更新下一次闹钟
        update_alarm_time();
        
        // 这里添加您的闹钟处理代码
    }
}