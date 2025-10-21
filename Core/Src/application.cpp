#include <stdlib.h>
#include <time.h>
#include <fake_unix/maxed_list.h>
#include <list>
#include <iic_sht35_driver.h>
#include <iic_ms5803_driver.h>
#include <serialport.h>
#include <calculate.h>
#include <timer.h>
#include <main.h>
#include <task_queue.h>
#include <stm32l4xx_ll_rtc.h>
#include <gpio_l4.h>
#include "lora.h"
#include "power_manager.h"
#include "protocol_manager.h"
#include "rtc.h"
#include "host.h"
time_t RTC_Get_Time(tm &time_struct);
bool statusxx = false;
task_queue *g_task_queue;
param_buffer *g_param_buffer;
extern "C" int _write(int fd, char *ptr, int len);
extern "C"
{
    extern int status;
    iic_sht45_driver *sht35;
    iic_ms5803_driver *ms5830;
    void (*iic_dma_rx_complete)(void *);
    void (*iic_dma_tx_complete)(void *);
    void (*iic_it_rx_complete)(void *, uint8_t);
    void (*iic_it_tx_complete)(void *);
    void *iic_current_device;
    extern int volatile ok;
}
void USART_Printf(USART_TypeDef *USARTx, const char *fmt, ...);

extern uint8_t wakeup_source;
class application
{
    iic_sht45_driver _sht35;
    task_queue _task_queue;
    param_buffer _param_buffer;
    iic_ms5803_driver _ms5830;
    timer _timer;
    Lora _lora;
    Host _host;
    calculate::param _params;
    calculate::result _res;
    int sss[2];
    PowerManager power_manager;
    // int       _led_status;
public:
    void host_enter_standby_mode()
    {
        // 检查Host是否接收到数据
        if (!_host.is_data_received() && !_host.standby_timer_canceled()) 
        {
            // printf("No data received within 10s, entering standby mode...\r\n");
            power_manager.enter_standby_mode();
        } 
        else 
        {
             _timer.cancel();
            // printf("Data received or timer canceled, skipping standby...\r\n");
        }
    }

    application() :_lora(LPUART1, serial_gpio(GPIOA, LL_GPIO_PIN_2), serial_gpio(GPIOA, LL_GPIO_PIN_3)),
    _host(USART1, serial_gpio(GPIOA, LL_GPIO_PIN_9), serial_gpio(GPIOA, LL_GPIO_PIN_10))
    {
        double CAP,RES;
        sht35 = &_sht35;
        sht35->sample_ok.connect(WRAP(sht35_sample_complete, this));
        _host.reset_receive_buffer();
        _lora.reset_receive_buffer();
        g_task_queue = &_task_queue;
        g_param_buffer = &_param_buffer;
        ms5830 = &_ms5830;
        ms5830->sample_complete.connect(WRAP(ms5803_sample_complete, this));
        // 把数据同步给protocol_manager
        ms5830->read_caldata();
        _ms5830.read();
        // internal_printf("aaaaa\r\n");
        // 启动Host通信（但不进入循环等待）
        _host.start_communication_prepare();
        // led_blink();
    }

    // void led_blink()
    // {
        // uint8_t current_hours = LL_RTC_TIME_GetHour(RTC);
        // uint8_t current_minutes = LL_RTC_TIME_GetMinute(RTC);
        // uint8_t current_seconds = LL_RTC_TIME_GetSecond(RTC);
  
        // printf("Current time: %02x:%02x:%02x\n", current_hours, current_minutes, current_seconds);
        // LL_GPIO_TogglePin(GPIOB,LL_GPIO_PIN_1);
        // read_ad_value();
        // _led_status++;
        // _timer.executate_after(1000, WRAP(led_blink, this));
    // }

    float read_ad_value()
    {
        // 启用内部稳压器
    LL_ADC_DisableDeepPowerDown(ADC1);
    LL_ADC_EnableInternalRegulator(ADC1);
    
    // 延长稳压器稳定时间
    uint32_t wait_loop_index = ((LL_ADC_DELAY_INTERNAL_REGUL_STAB_US * (SystemCoreClock / (100000 * 2))) / 10) * 2;
    while(wait_loop_index != 0) {
        wait_loop_index--;
    }

    // 确保ADC已启用
    if (!LL_ADC_IsEnabled(ADC1)) {
        LL_ADC_Enable(ADC1);
        // 等待ADC准备好
        while (LL_ADC_IsActiveFlag_ADRDY(ADC1) == 0) {}
    }
    
    uint16_t adc_value = 0;

    // 启动ADC转换
    LL_ADC_REG_StartConversion(ADC1);
    
    // 等待转换完成
    while (LL_ADC_IsActiveFlag_EOC(ADC1) == 0) {
        // 可选：添加超时处理
    }
    
    // 读取转换结果
    // for(uint8_t i=0;i<5;i++)
    adc_value = LL_ADC_REG_ReadConversionData12(ADC1);
    
    // 清除EOC标志
    LL_ADC_ClearFlag_EOC(ADC1);

    __IO uint16_t voltage = __LL_ADC_CALC_DATA_TO_VOLTAGE((uint32_t)2000,adc_value,LL_ADC_RESOLUTION_12B);
    
    // printf("uint16 voltage:%d\r\n",voltage);
    float actual_voltage = voltage/1000.0 * 2 *(1.77/1.03);
    // printf("voltage:%.2f\r\n",actual_voltage);
    return actual_voltage;
    }

    void sht35_sample_complete(float tem, float hum)
    {
        _params.Sht30Temperature = tem;
        _params.Sht30Humidity = hum;
        _params.Ms5803Temperature = tem;
        _params.SF6_percentage = 1;
        calculate::calculate_sf6(_res, _params);
        // 协议数据同步
        // 获取一次device id
        float voltage = read_ad_value();
        _lora.get_protocol_manager().set_battery_voltage(voltage);
        _lora.get_protocol_manager().set_temperature(_res.Tempture);
        _lora.get_protocol_manager().set_humidity(_res.RH_f);
        _lora.get_protocol_manager().set_density(_res.density);
        _lora.get_protocol_manager().set_P20(_res.P20);
        _lora.get_protocol_manager().set_ppm(_res.PPMS);
        _lora.get_protocol_manager().set_dew_point(_res.Point);
        _lora.get_protocol_manager().set_pressure(_res.Press);
        // _lora.get_protocol_manager().set_device_id(get_history_config()->device_id);

        // Host模块数据同步 - 新增
        _host.get_protocol_manager().set_battery_voltage(voltage);
        _host.get_protocol_manager().set_temperature(_res.Tempture);
        _host.get_protocol_manager().set_humidity(_res.RH_f);
        _host.get_protocol_manager().set_density(_res.density);
        _host.get_protocol_manager().set_P20(_res.P20);
        _host.get_protocol_manager().set_ppm(_res.PPMS);
        _host.get_protocol_manager().set_dew_point(_res.Point);
        _host.get_protocol_manager().set_pressure(_res.Press);
        // _host.get_protocol_manager().set_device_id(get_history_config()->device_id);
        // printf("wakeup_source:%d\r\n",wakeup_source);
        switch (wakeup_source)
        {
        case 0:
            power_manager.enter_standby_mode();
            // _lora.start_upload_process(); // 开始进行主动上报
            break;
        case 1:
            _lora.get_protocol_manager().set_wake_type("lora");
            _timer.executate_after(500,WRAP(enter_standby_mode,&power_manager));
            break;
        case 2:
            _lora.get_protocol_manager().set_wake_type("alarm");
            _host.get_protocol_manager().set_wake_type("alarm");
            _lora.upload_data();
            update_alarm_time();
            power_manager.enter_standby_mode();
            break;
        case 3:
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_1);
            _timer.executate_after(10000, WRAP(host_enter_standby_mode, this));
            break;
        }
    }

    void ms5803_sample_complete(float temperature, float pressure)
    {
        _params.Ms5803Pressure = pressure / 100000;
        if(_params.Ms5803Pressure > 1.5)
            _params.Ms5803Pressure = _params.Ms5803Pressure - 1.01325;
        else
            _params.Ms5803Pressure = _params.Ms5803Pressure;
        _params.Ms5803Temperature = temperature;
        sht35->read();
    }
    void exec()
    {
        if (!g_task_queue->empty())
            g_task_queue->invoke();
    }
};

application *app;
extern "C"
{
    int initial_application(void)
    {
        if(!app)
            app = new application;
        return 0;
    }
    void exec()
    {
        app->exec();
    }
}