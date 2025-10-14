#ifndef HOST_H
#define HOST_H

#include "serialport.h"
#include "protocol_manager.h"
#include "stm32l4xx_ll_gpio.h"
class Host : public serialport
{
public:
    Host(USART_TypeDef *usart, const serial_gpio &tx, const serial_gpio &rx);
    
    void start_communication_prepare();  // 准备模式
    void check_and_enter_wait_loop();    // 检查并进入等待循环
    void on_recv(uint8_t ch) override;
    
    // 添加协议管理器访问方法
    ProtocolManager& get_protocol_manager() { return protocol_manager; }
    
    // 添加数据接收标志
    bool is_data_received() const { return data_received_; }
    void cancel_standby_timer() { standby_timer_canceled_ = true; }
    bool standby_timer_canceled() const { return standby_timer_canceled_; }
    void reset_receive_buffer();
private:
    void start_protocol_handle_process();
    uint8_t recv_buffer_[512];
    uint16_t count_;
    ProtocolManager protocol_manager;
    bool processing_command_;
    bool data_received_;           // 数据接收标志
    bool standby_timer_canceled_;  // 休眠定时器取消标志
    bool in_wait_loop_;            // 是否在循环等待模式中
};

#endif // HOST_H