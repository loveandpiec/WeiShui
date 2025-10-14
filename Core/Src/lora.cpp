#include "lora.h"
#include "task_queue.h"
#include <cstring>
#include "malloc.h"
#include "stm32l4xx_ll_utils.h"
void Lora::start_protocol_handle_process()
{
    // 进行协议解析流程
    // printf("start protocol handle\r\n");
    // printf("完整响应: %s",recv_buffer_);
    // 获取lora模块接收到的数据
    char* response = protocol_manager.lora_protocol_json_handle((const char *)recv_buffer_);
    // printf("response:%s",response);
    if(response)
    {
        write(response,strlen(response));
        free(response);
        response = nullptr;
    }
}

void Lora::upload_data()
{
    // 1.根据协议准备透传数据
    // 2.上传数据
    // printf("start upload data\r\n");
    char*response = protocol_manager.generate_lora_protocol_json("0",nullptr,nullptr);
    if(response)
    {
        write(response,strlen(response));
        free(response); // 释放cJSON分配的内存
    }
}

Lora::Lora(USART_TypeDef *usart, const serial_gpio &tx, const serial_gpio &rx):serialport(usart,tx,rx),count_(0)
{
    timer_ = new timer();
}

void remove_crlf(char *str) {
    char *p = str;
    while (*p) {
        if (*p == '\r' || *p == '\n') {
            memmove(p, p + 1, strlen(p));
        } else {
            p++;
        }
    }
}

void Lora::reset_receive_buffer()
{
    memset(recv_buffer_, 0, sizeof(recv_buffer_));
    count_ = 0;
}
void Lora::on_recv(uint8_t ch)
{
    recv_buffer_[count_++] = ch;
    count_ &= 0x1ff;
    // printf("完整响应: %s",recv_buffer_);
    // printf((char*)ch);
    if(ch == '\n' && count_>=5) 
    {
        internal_printf("完整响应[%d字节]: %s", count_, recv_buffer_);
        recv_buffer_[count_] = '\0';  // 确保字符串终止
        if(strstr((char*)recv_buffer_,"{")&&(strstr((char*)recv_buffer_,"}")))
        {
            start_protocol_handle_process();
        }
        else if(strstr((char*)recv_buffer_,"\r\nOK\r\n") || strstr((char*)recv_buffer_,"\r\nER00\r\n"))
        {
            char* response = protocol_manager.wrap_lora_response_json("6",(char*)recv_buffer_);
            if(response)
            {
                for (size_t i = 0; i < strlen(response); i++)
                {
                    LL_USART_TransmitData8(USART1, reinterpret_cast<const uint8_t *>(response)[i]);
                    while (!LL_USART_IsActiveFlag_TC(USART1));
                }
                free(response);
                response = nullptr;
            }
        }
        reset_receive_buffer();
    }
}

