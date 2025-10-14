#include "host.h"
#include "task_queue.h"
#include <cstring>
#include "malloc.h"
#include "stm32l4xx_ll_utils.h"
#include "rtc.h"
void Host::start_communication_prepare()
{
    // printf("Host communication prepared, waiting for commands (10s timeout)...\r\n");
    
    // 初始化串口
    // initialize_serial();
    
    // 初始化状态
    reset_receive_buffer();
    processing_command_ = false;
    data_received_ = false;
    standby_timer_canceled_ = false;
    in_wait_loop_ = false;
    
    // 设置设备ID
    protocol_manager.set_device_id(get_history_config()->device_id);
    
    // printf("Host prepared, device ID: %d. Will enter standby in 10s if no data received.\r\n", protocol_manager.get_device_id());
}

void Host::check_and_enter_wait_loop()
{
    // 这个方法在application的主循环中调用
    if (data_received_ && !in_wait_loop_) {
        // printf("=== Data received, entering wait loop mode ===\r\n");
        in_wait_loop_ = true;
        standby_timer_canceled_ = true;
    }
}
void Host::start_protocol_handle_process()
{
    // 设置处理标志，防止重入
    if (processing_command_) {
        // printf("Already processing command, skipping...\r\n");
        return;
    }
    processing_command_ = true;
    
    // 标记已接收到数据
    data_received_ = true;
    
    // 进行协议解析流程
    // printf("Received command: %s", recv_buffer_);
    
    // 使用协议管理器处理接收到的数据
    char* response = protocol_manager.lora_protocol_json_handle((const char *)recv_buffer_);
    
    if(response)
    {
        // printf("Response: %s", response);
        
        // 发送响应回上位机
        write(response, strlen(response));
        
        // 释放内存
        free(response);

        response = nullptr;
    }
    else
    {
        // printf("No response generated for command\r\n");
    }
    
    // 清空接收缓冲区，准备接收下一条命令
    reset_receive_buffer();
    processing_command_ = false;
    
    // printf("Ready for next command...\r\n");
}

void Host::reset_receive_buffer()
{
    memset(recv_buffer_, 0, sizeof(recv_buffer_));
    count_ = 0;
}

Host::Host(USART_TypeDef *usart, const serial_gpio &tx, const serial_gpio &rx)
    : serialport(usart, tx, rx), count_(0), processing_command_(false), 
      data_received_(false), standby_timer_canceled_(false), in_wait_loop_(false)
{
    reset_receive_buffer();
}

void Host::on_recv(uint8_t ch)
{
    // 如果正在处理命令，忽略新数据
    if (processing_command_) {
        return;
    }
    
    // 检查缓冲区边界
    if (count_ >= (sizeof(recv_buffer_) - 1)) {
        // printf("Receive buffer overflow, resetting...\r\n");
        reset_receive_buffer();
        return;
    }
    
    // 将接收到的字符存入缓冲区
    recv_buffer_[count_++] = ch;
    
    // 检测到换行符且有一定数据长度时开始处理
    if (ch == '\n' && count_ >= 5) 
    {
        // 确保字符串正确终止
        recv_buffer_[count_] = '\0';
        
        // printf("Complete message received: %s", recv_buffer_);
        
        // 检查是否是有效的JSON格式命令
        if (strstr((char*)recv_buffer_, "{") && strstr((char*)recv_buffer_, "}"))
        {
            if(strstr((char*)recv_buffer_,"lora"))
            {
                // internal_printf("Valid JSON detected, starting protocol handling...\r\n");
                char*response_msg = protocol_manager.parse_host_at_command((char*)recv_buffer_);
                if(response_msg)
                {
                    printf("data prepare to send to lora%s\r\n",response_msg);
                    for (size_t i = 0; i < strlen(response_msg); i++)
                    {
                        LL_USART_TransmitData8(LPUART1, reinterpret_cast<const uint8_t *>(response_msg)[i]);
                        while (!LL_USART_IsActiveFlag_TC(LPUART1));
                    }
                    free(response_msg);
                    response_msg = nullptr;
                    reset_receive_buffer();
                }
            }
            else
                start_protocol_handle_process();
        }
        else
        {
            // 如果不是JSON格式，清空缓冲区继续等待
            printf("Invalid command format, clearing buffer\r\n");
            reset_receive_buffer();
        }
    }
    
    // 额外的缓冲区溢出保护
    if (count_ >= (sizeof(recv_buffer_) - 1))
    {
        printf("Buffer overflow protection triggered\r\n");
        reset_receive_buffer();
    }
}