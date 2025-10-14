#pragma once
#include <stdint.h>
#include <task_queue.h>
#include <timer.h>

struct iic_ms5803_driver
{
private:
    volatile int _finish_tx;
    volatile int _finish_rx;
    char _buffer[3];
    char _send_buffer[4];
    uint8_t _current_rx_count;
    uint16_t cali_para[7];
    void read_adc();
    enum reading_task
    {
        PRESS=0,
        PRESS_ADC=1,
        TEMP=2,
        TEMP_ADC=3        
    }_current_reading_task;

    timer _timer;
    void common();
    void send_command_word();
    uint32_t pressure_ad;
    uint32_t temperature_ad;
    signal<void()> ad_sample_complete;
    void on_sample_completed();
public:
   
    enum const_values
    {
        MS5803_DEVICE_ADDRESS=0x77,	//CSB 0=0x77 1=0x76
        MS5803_RESET_CMD=0x1E,
        MS5803_PROM_RD_CMD=0xA0,	//A0->AE
        MS5803_CMD_ADC_READ	=0x00,
        MS5803_CMD_ADC_CONV=0x40,
        MS5803_CMD_ADC_D1=0x00,
        MS5803_CMD_ADC_D2=0x10,
        MS5803_CMD_ADC_4096=0x08
    };
    iic_ms5803_driver();
    void read();
    void read_caldata();
    // void dma_tx_complete_handle();
    // void dma_rx_complete_handle();
    void iic_rx_complete_handle(uint8_t ch);
    void iic_tx_complete_handle();
    signal<void(float,float)> sample_complete;
};
