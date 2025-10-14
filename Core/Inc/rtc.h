#ifndef RTC_H__
#define RTC_H__
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"

#ifdef __cplusplus
extern "C" {
#endif


// Flash配置数据结构
typedef struct {
    uint32_t init_flag;
    uint32_t alarm_interval;
    uint32_t device_id;
} History_Config_T;

/**
  * @brief  从Flash读取RTC配置
  */
History_Config_T* get_history_config(void);
void RTC_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_Set_Date(uint8_t year, uint8_t month, uint8_t day);
void MX_RTC_Init(void);
void Set_RTC_Alarm_LL(uint8_t h,uint8_t m,uint8_t s);
void update_alarm_time();
void store_history_config(const History_Config_T* config);

// void store_alarm_interval(uint32_t alarm_interval);

// uint32_t get_alarm_interval();
uint32_t WaitForSynchro_RTC(void);
#ifdef __cplusplus
}
#endif

#endif