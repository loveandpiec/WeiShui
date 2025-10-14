#ifndef POWER_MANGAER_H__
#define POWER_MANGAER_H__
#include <cstring>
#include <stdio.h>
#include "stm32l412xx.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_pwr.h"
// 定义时间编码宏
#define ENCODE_TIME(h, m, s) (((h) << 16) | ((m) << 8) | (s))
class PowerManager
{
public:
    PowerManager() = default;
    ~PowerManager() = default;
    void deinit();
    void usart1_deinit();
    void lpuart_deinit();
    void I2C1_DeInit();
    void gpio_deinit();
    void enter_standby_mode();
};
#endif