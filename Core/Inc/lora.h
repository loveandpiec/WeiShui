#pragma once

#include "timer.h"
#include "stm32l4xx_ll_usart.h"
#include "serialport.h"
#include "power_manager.h"
#include "protocol_manager.h"

class Lora:public serialport
{
private:
    timer* timer_;
    timer recv_timer_;
    uint8_t recv_buffer_[256]={0};
    uint8_t count_;
    PowerManager power_manager;
    ProtocolManager protocol_manager;
public:
    Lora(USART_TypeDef* usart,const serial_gpio& tx,const serial_gpio& rx);
    ~Lora()
    {    
        delete timer_;
        timer_ = nullptr;
    };
    void start_protocol_handle_process();
    void upload_data();
    void on_recv(uint8_t ch) override;
    void reset_receive_buffer();
    ProtocolManager& get_protocol_manager(){return protocol_manager;}
};
