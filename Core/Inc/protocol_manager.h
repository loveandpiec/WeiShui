#ifndef PROTOCOL_MANAGER_H__
#define PROTOCOL_MANAGER_H__

#include "cJSON.h"
#include <stdio.h>
#include <cstring>
#include "power_manager.h"
// 日志缓冲区
static const int LOG_BUFFER_SIZE = 1024;
extern char log_buffer_[LOG_BUFFER_SIZE];
extern int log_buffer_pos_;
// 内部打印函数
void internal_printf(const char* format, ...);

// 清空日志
void clear_log();

class ProtocolManager
{
public:
    ProtocolManager()=default;
    ~ProtocolManager() = default;
    char* lora_protocol_json_handle(const char* recv_msg);
    char*generate_lora_protocol_json(const char* msg_id,const char* request,cJSON* param_list);
    /**
     * @brief 解析上位机发送的单个LORA AT指令JSON
     * @param recv_msg 接收到的JSON消息
     * @return AT指令字符串，需要调用者释放内存，NULL表示解析失败
     */
    char* parse_host_at_command(const char* recv_msg);
    /**
     * @brief 包装LORA模块返回的结果为JSON格式
     * @param msg_id 消息ID
     * @param lora_response LORA模块返回的原始结果
     * @return JSON格式的字符串，需要调用者释放内存
     */
    char *wrap_lora_response_json(const char* msg_id, const char* lora_response);

public:
    inline char* get_wake_type(void){return wake_type;}
    inline void set_wake_type(char* wake_type){memcpy(this->wake_type,wake_type,strlen(wake_type));}
    inline void set_temperature(float temperature){this->temperature = temperature;}
    inline float get_temperature(void){return this->temperature;}
    inline void set_humidity(float humidity){this->humidity = humidity;}
    inline float get_humidity(void){return this->humidity;}
    inline void set_P20(float P20){this->P20 = P20;}
    inline float get_P20(void){return this->P20;}
    inline void set_dew_point(float dew_point){this->dew_point = dew_point;}
    inline float get_dew_point(void){return this->dew_point;}
    inline void set_ppm(float ppm){this->ppm = ppm;}
    inline float get_ppm(void){return this->ppm;}
    inline void set_density(float density){this->density = density;}
    inline float get_density(void){return this->density;}
    inline void set_battery_voltage(float battery_voltage){this->battery_voltage = battery_voltage;}
    inline float get_batter_voltage(void){return this->battery_voltage;}
    inline void set_pressure(float pressure){this->pressure = pressure;}
    inline float get_pressure(void){return this->pressure;}
    // inline void set_device_id(uint32_t device_id){this->device_id = device_id;}
    // inline uint32_t get_device_id(){return device_id;}
private:
    char wake_type[20]{0};
    // uint32_t device_id;
    float pressure = 0.0;
    float temperature = 0.0;
    float humidity = 0.0;
    float P20 = 0.0;
    float dew_point = 0.0;
    float ppm = 0.0;
    float density = 0.0;
    float battery_voltage = 0.0;
    PowerManager _power_manager;
};

#endif