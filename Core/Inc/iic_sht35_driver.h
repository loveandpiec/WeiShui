#pragma once
#include <stdint.h>
#include <task_queue.h>
#include <timer.h>

struct iic_sht45_driver
{
private:
    volatile int _finish_tx;
    volatile int _finish_rx;
    char _buffer[6];
    char _send_buffer[2];
    uint8_t _current_count;
    timer _timer;
public:
    iic_sht45_driver();
    void read();
    void read_value();
    // void dma_tx_complete_handle();
    // void dma_rx_complete_handle();
    void iic_rx_complete_handle(uint8_t ch);
    void iic_tx_complete_handle();
    signal<void(float,float)> sample_ok;
};
