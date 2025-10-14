#include "power_manager.h"
#include <cstdio>
#include "main.h"
extern "C"
{
    extern void MX_RTC_Init(void);

    extern void Set_RTC_Alarm_LL(uint8_t h,uint8_t m,uint8_t s);
}

void PowerManager::deinit()
{
    usart1_deinit();
    lpuart_deinit();
    I2C1_DeInit();
    gpio_deinit();
}

void PowerManager::usart1_deinit()
{
    // 1. 等待传输完成
    if(LL_USART_IsEnabled(USART1))
    {
        while(LL_USART_IsActiveFlag_TC(USART1) == 0)
        {
            // 等待传输完成标志
        }
    }
    
    // 2. 禁用USART1
    LL_USART_Disable(USART1);
    
    // 3. 禁用所有中断
    // LL_USART_DisableIT_NE(USART1);
    // LL_USART_DisableIT_TXE(USART1);
    LL_USART_DisableIT_TC(USART1);
    LL_USART_DisableIT_PE(USART1);
    LL_USART_DisableIT_ERROR(USART1);
    
    // 4. 清除所有状态标志
    LL_USART_ClearFlag_TC(USART1);
    // LL_USART_ClearFlag_RXNE(USART1);
    // LL_USART_ClearFlag_TXE(USART1);
    LL_USART_ClearFlag_ORE(USART1);
    LL_USART_ClearFlag_NE(USART1);
    LL_USART_ClearFlag_FE(USART1);
    LL_USART_ClearFlag_PE(USART1);
    
    // 5. 禁用NVIC中断
    NVIC_DisableIRQ(USART1_IRQn);
    
    // 6. 复位USART1外设
    LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_USART1);
    LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_USART1);
    
    // 7. 禁用USART1时钟
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);
    
    // 8. 配置GPIO为模拟输入（最低功耗）
    // USART1_TX: PA9, USART1_RX: PA10
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_NO);
}

/**
  * @brief  取消初始化I2C1外设
  * @param  无
  * @retval 无
  */
void PowerManager::I2C1_DeInit(void)
{
  /* 禁用I2C1外设 */
  LL_I2C_Disable(I2C1);
  
  /* 清除所有标志位 */
  LL_I2C_ClearFlag_STOP(I2C1);
  LL_I2C_ClearFlag_BERR(I2C1);
  LL_I2C_ClearFlag_ARLO(I2C1);
  LL_I2C_ClearFlag_OVR(I2C1);

  /* 复位I2C1外设 */
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);
  
  /* 禁用I2C1时钟 */
  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* 复位I2C1 GPIO引脚 (PB6: SCL, PB7: SDA) */
  LL_AHB2_GRP1_ForceReset(LL_AHB2_GRP1_PERIPH_GPIOB);
  LL_AHB2_GRP1_ReleaseReset(LL_AHB2_GRP1_PERIPH_GPIOB);
  
  /* 或者单独配置GPIO为模拟输入（高阻态）*/
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ANALOG);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ANALOG);
  
  /* 关闭GPIOB时钟（如果不再使用其他PB引脚）*/
  // LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
}


void PowerManager::lpuart_deinit()
{
    // 1. 等待传输完成
    if(LL_LPUART_IsEnabled(LPUART1))
    {
        while(LL_LPUART_IsActiveFlag_TC(LPUART1) == 0)
        {
            // 等待传输完成标志
        }
    }

    // 禁用外设以降低功耗
    // 先禁用LPUART
    LL_LPUART_Disable(LPUART1);

    // 清除所有中断标志
    LL_LPUART_ClearFlag_TC(LPUART1);
    LL_LPUART_ClearFlag_ORE(LPUART1);
    LL_LPUART_ClearFlag_NE(LPUART1);
    LL_LPUART_ClearFlag_FE(LPUART1);
    LL_LPUART_ClearFlag_PE(LPUART1);

    // 禁用所有中断
    LL_LPUART_DisableIT_TC(LPUART1);
    LL_LPUART_DisableIT_RXNE(LPUART1);
    LL_LPUART_DisableIT_TXE(LPUART1);
    LL_LPUART_DisableIT_PE(LPUART1);
    LL_LPUART_DisableIT_ERROR(LPUART1);

    // 禁用NVIC中断
    NVIC_DisableIRQ(LPUART1_IRQn);

    // 禁用LPUART时钟
    LL_APB1_GRP2_DisableClock(LL_APB1_GRP2_PERIPH_LPUART1);  // 注意：LPUART1通常在APB1GRP2

    // 6. 复位LPUART外设
    // 注意：不同系列的LPUART复位方式可能不同
    LL_APB1_GRP2_ForceReset(LL_APB1_GRP2_PERIPH_LPUART1);
    LL_APB1_GRP2_ReleaseReset(LL_APB1_GRP2_PERIPH_LPUART1);
    // 7. 禁用LPUART时钟
    // 注意：不同系列的LPUART时钟使能方式可能不同
    LL_APB1_GRP2_DisableClock(LL_APB1_GRP2_PERIPH_LPUART1);
    // 8. 配置GPIO为模拟输入
    // 根据实际硬件连接配置GPIO引脚
    // 例如：LPUART1_TX: PA2, LPUART1_RX: PA3（根据具体型号调整）
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_2, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_3, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_2, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_3, LL_GPIO_PULL_NO);
}

void PowerManager::gpio_deinit()
{
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    // 将所有引脚设置成模拟，除了PA0
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4|LL_GPIO_PIN_7|LL_GPIO_PIN_8|LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void PowerManager::enter_standby_mode()
{   
    // 1. 确保所有操作完成
    __DSB();
    __ISB();
    
    // 2. 简单的延时确保稳定性
    LL_mDelay(50);
    
    // 3. 配置唤醒引脚（保持简单）
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN1);  // PA0
    LL_PWR_SetWakeUpPinPolarityLow(LL_PWR_WAKEUP_PIN1); // 根据实际硬件选择
    
    // 4. 清除所有标志
    LL_PWR_ClearFlag_WU();
    LL_PWR_ClearFlag_SB();
    
    // 5. 简单的LED指示
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_1);
    LL_mDelay(100);
    // printf("Entering standby mode now...\r\n");
    deinit();
    // 6. 进入Standby模式
    LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);
    LL_LPM_EnableDeepSleep();
    
    // 确保指令完成
    __DSB();
    __ISB();
    
    // 等待中断进入Standby
    __WFI();
    
    // 这行代码不会被执行
    // printf("ERROR: Should not reach here!\r\n");

}
