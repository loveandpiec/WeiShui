#include "protocol_manager.h"
#include "rtc.h"
#include "cJSON.h"
#include <stdarg.h>
char log_buffer_[LOG_BUFFER_SIZE]{0};
int log_buffer_pos_ = 0;
char *ProtocolManager::lora_protocol_json_handle(const char *recv_msg)
{
    // 清空之前的日志
    clear_log();
    
    internal_printf("lora_protocol_json_handle called\r\n");
    internal_printf("recv_msg: %s\r\n", recv_msg);
    // printf("recv_msg: %s\r\n", recv_msg);

    // 获取root
    cJSON* json_root = cJSON_Parse(recv_msg);
    if(!json_root) 
    {
        internal_printf("can't parse json_root is nullptr\r\n");
        // printf("can't parse json_root is nullptr\r\n");
        _power_manager.enter_standby_mode();
    }

    // 获取msg_id
    cJSON* msg_id = cJSON_GetObjectItem(json_root,"msg_id");

    if(!msg_id || !cJSON_IsString(msg_id)) 
    {
        internal_printf("can't parse msg_id is nullptr or not string\r\n");
        // printf("can't parse msg_id is nullptr or not string\r\n");
        cJSON_Delete(json_root);
        _power_manager.enter_standby_mode();
    }
    internal_printf("msg_id:%s\n",msg_id->valuestring);
    // printf("msg_id:%s\n",msg_id->valuestring);


    cJSON*cjson_device_id = cJSON_GetObjectItem(json_root,"device_id");
    internal_printf("self device_id:%s,recv device_id:%s",get_history_config()->device_id,cjson_device_id->valueint);
    if(!cjson_device_id || !cJSON_IsNumber(cjson_device_id)|| get_history_config()->device_id != cjson_device_id->valueint)
    {
        internal_printf("can't parse device_id is nullptr or not number\r\n");
        // printf("can't parse device_id is nullptr or not number\r\n");
        cJSON_Delete(json_root);
        //  直接进休眠
        _power_manager.enter_standby_mode();
    }
    char* result = nullptr;
    if(strstr(msg_id->valuestring,"0"))
        result = generate_lora_protocol_json(msg_id->valuestring, nullptr, nullptr);
    else
    {
        // 获取request
        cJSON* request = cJSON_GetObjectItem(json_root,"request");
        if(!request || !cJSON_IsString(request)) 
        {
            internal_printf("can't parse request is nullptr or not string\r\n");
            cJSON_Delete(json_root);
            _power_manager.enter_standby_mode();
        }
        internal_printf("request:%s\n",request->valuestring);

        // 获取param_list - 这里应该是数组而不是字符串
        cJSON* param_list = cJSON_GetObjectItem(json_root,"param_list");
        if(!param_list || !cJSON_IsArray(param_list)) 
        {
            internal_printf("can't parse param_list is nullptr or not array\r\n");
            cJSON_Delete(json_root);
            _power_manager.enter_standby_mode();
        }

        // 打印param_list内容
        int param_count = cJSON_GetArraySize(param_list);
        result = generate_lora_protocol_json(msg_id->valuestring, request->valuestring, param_list);
    }    
    
    // 清理内存
    cJSON_Delete(json_root);
    return result;
}

// 传入会话id和请求体
char *ProtocolManager::generate_lora_protocol_json(const char* msg_id,const char* request,cJSON* param_list)
{
    cJSON* root = cJSON_CreateObject(); // 创建根对象
    if(!msg_id)
    {
        cJSON_AddStringToObject(root,"msg_id","");
        return cJSON_Print(root);
    }
    cJSON_AddStringToObject(root,"msg_id",msg_id);

    cJSON_AddNumberToObject(root,"device_id",get_history_config()->device_id);

    if(strstr(msg_id,"0"))
    {
        char buffer[10]{0};
        // 数据保留2位
        // 主动上报，添加主动上报数据到日记
        cJSON* log = cJSON_CreateObject();
        
        sprintf(buffer,"%.2f",temperature);
        cJSON_AddStringToObject(log,"temperature",buffer);
        
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%.2f",pressure);
        cJSON_AddStringToObject(log,"pressure",buffer);

        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%.2f",humidity);
        cJSON_AddStringToObject(log,"humidity",buffer);

        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%.2f",P20);
        cJSON_AddStringToObject(log,"P20",buffer);

        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%.2f",dew_point);
        cJSON_AddStringToObject(log,"dew_point",buffer);

        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%.2f",ppm);
        cJSON_AddStringToObject(log,"ppm",buffer);

        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%.2f",density);
        cJSON_AddStringToObject(log,"density",buffer);

        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%.2f",battery_voltage);
        cJSON_AddStringToObject(log,"battery_voltage",buffer);

        cJSON_AddNumberToObject(log,"wake_interval",get_history_config()->alarm_interval);

        // 等待RTC同步
        WaitForSynchro_RTC();
        
        // 获取当前时间（BCD格式）
        uint8_t year = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));
        uint8_t month = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
        uint8_t date = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
        uint8_t hours = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
        uint8_t minutes = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
        uint8_t seconds = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
        
        char calendar_buffer[50]{0};
        sprintf(calendar_buffer,"20%02d-%02d-%02d %02d:%02d:%02d",year,month,date,hours,minutes,seconds);
        cJSON_AddStringToObject(log,"calendar",calendar_buffer);

        cJSON_AddStringToObject(log,"wake_type",wake_type);
        cJSON_AddItemToObject(root,"log",log);
    }   
    else
    {
        if(!request) 
        {
            cJSON_AddStringToObject(root,"request","");
            return cJSON_Print(root); // 如果请求体是个空的，直接组装空的
        }
        if(strstr(request,"get")) // 获取值操作
        {
            char buffer[10]{0};
            // 创建一个字典数组
            cJSON*result_list = cJSON_CreateArray();
            // 获取valuestring
            cJSON* json_item = nullptr;
            cJSON_ArrayForEach(json_item,param_list)
            {
                if(!json_item->valuestring) continue;
                    
                if(strstr(json_item->valuestring,"temperature"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",temperature);
                    cJSON_AddStringToObject(dict,"temperature",buffer);
                    // cJSON_AddNumberToObject(dict,"temperature",temperature);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"pressure"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",pressure);
                    cJSON_AddStringToObject(dict,"pressure",buffer);
                    // cJSON_AddNumberToObject(dict,"pressure",pressure);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"humidity"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",humidity);
                    cJSON_AddStringToObject(dict,"humidity",buffer);
                    // cJSON_AddNumberToObject(dict,"humidity",humidity);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"P20"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",P20);
                    cJSON_AddStringToObject(dict,"P20",buffer);
                    // cJSON_AddNumberToObject(dict,"P20",P20);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"dew_point"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",dew_point);
                    cJSON_AddStringToObject(dict,"dew_point",buffer);
                    // cJSON_AddNumberToObject(dict,"dew_point",dew_point);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"ppm"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",ppm);
                    cJSON_AddStringToObject(dict,"ppm",buffer);
                    // cJSON_AddNumberToObject(dict,"ppm",ppm);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"density"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",density);
                    cJSON_AddStringToObject(dict,"density",buffer);
                    // cJSON_AddNumberToObject(dict,"density",density);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"battery_voltage"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    memset(buffer,0,sizeof(buffer));
                    sprintf(buffer,"%.2f",battery_voltage);
                    cJSON_AddStringToObject(dict,"battery_voltage",buffer);
                    // cJSON_AddNumberToObject(dict,"battery_voltage",battery_voltage);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"wake_type"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    cJSON_AddStringToObject(dict,"wake_type",wake_type);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"wake_interval"))
                {
                    cJSON*dict = cJSON_CreateObject();
                    cJSON_AddNumberToObject(dict,"wake_interval",get_history_config()->alarm_interval);
                    cJSON_AddItemToArray(result_list,dict);
                }

                if(strstr(json_item->valuestring,"calendar"))
                {
                    cJSON*dict = cJSON_CreateObject();
                           // 等待RTC同步
                    WaitForSynchro_RTC();
                    
                    // 获取当前时间（BCD格式）
                    uint8_t year = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetYear(RTC));
                    uint8_t month = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetMonth(RTC));
                    uint8_t date = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_DATE_GetDay(RTC));
                    uint8_t hours = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetHour(RTC));
                    uint8_t minutes = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetMinute(RTC));
                    uint8_t seconds = __LL_RTC_CONVERT_BCD2BIN(LL_RTC_TIME_GetSecond(RTC));
                    
                    char calendar_buffer[50]{0};
                    sprintf(calendar_buffer,"20%02d-%02d-%02d %02d:%02d:%02d",year,month,date,hours,minutes,seconds);
                    cJSON_AddStringToObject(dict,"calendar",calendar_buffer);
                    cJSON_AddItemToArray(result_list,dict);
                }
            }
            cJSON_AddItemToObject(root,"result",result_list);
        }
        else if(strstr(request,"set")) // 设置操作
        {
            // 获取valuestring
            cJSON* json_item = nullptr;
            cJSON_ArrayForEach(json_item,param_list)
            {
                if(!json_item)
                {
                    // printf("not found json_item!\r\n");
                }

                /*唤醒时间设置*/
                cJSON* json_wake_interval_obj = cJSON_GetObjectItem(json_item,"wake_interval");
                if(json_wake_interval_obj && cJSON_IsNumber(json_wake_interval_obj))
                {
                    // printf("set wake_interval:%d\r\n", json_wake_interval_obj->valueint);
                    History_Config_T config;
                    // 数据同步
                    config.init_flag = get_history_config()->init_flag;
                    config.device_id = get_history_config()->device_id;
                    config.alarm_interval = json_wake_interval_obj->valueint;
                    store_history_config(&config);
                    update_alarm_time();
                }

                /*设备id设置*/
                cJSON* json_device_id = cJSON_GetObjectItem(json_item,"device_id");
                if(json_device_id&&cJSON_IsNumber(json_device_id))
                {
                    History_Config_T config;
                    // 数据同步
                    config.init_flag = get_history_config()->init_flag;
                    config.device_id = json_device_id->valueint;
                    config.alarm_interval = get_history_config()->alarm_interval;
                    store_history_config(&config);
                }

                /*日历设置*/
                cJSON* calendar_array = cJSON_GetObjectItem(json_item,"calendar");
                if(calendar_array&&cJSON_IsArray(calendar_array)&&cJSON_GetArraySize(calendar_array)==6)
                {
                    LL_RTC_DisableWriteProtection(RTC);
                    LL_RTC_EnableInitMode(RTC);
                    uint32_t year = __LL_RTC_CONVERT_BIN2BCD(cJSON_GetArrayItem(calendar_array,0)->valueint);
                    uint32_t month = __LL_RTC_CONVERT_BIN2BCD(cJSON_GetArrayItem(calendar_array,1)->valueint);
                    uint32_t day = __LL_RTC_CONVERT_BIN2BCD(cJSON_GetArrayItem(calendar_array,2)->valueint);
                    uint32_t hour = __LL_RTC_CONVERT_BIN2BCD(cJSON_GetArrayItem(calendar_array,3)->valueint);
                    uint32_t minute = __LL_RTC_CONVERT_BIN2BCD(cJSON_GetArrayItem(calendar_array,4)->valueint);
                    uint32_t second = __LL_RTC_CONVERT_BIN2BCD(cJSON_GetArrayItem(calendar_array,5)->valueint);
                    LL_RTC_DATE_Config(RTC,LL_RTC_WEEKDAY_MONDAY,day,month,year);
                    LL_RTC_TIME_Config(RTC,LL_RTC_TIME_FORMAT_AM_OR_24,hour,minute,second);
                    LL_RTC_DisableInitMode(RTC);
                    LL_RTC_EnableWriteProtection(RTC);
                    LL_RTC_TIME_Get(RTC);
                    LL_RTC_DATE_Get(RTC);
                }
            }
            // 增加debug调试信息
            cJSON_AddStringToObject(root,"result","ok");
        }
    }

    // 添加debug信息
    const char*debug_log = log_buffer_;
    if(strlen(debug_log) > 0)
    {
        cJSON_AddStringToObject(root,"debug",debug_log);
        internal_printf("Added debug log to JSON, length: %d\r\n", strlen(debug_log));
    }

    char* result = cJSON_Print(root);
    cJSON_Delete(root);
    return result;
}

#include <malloc.h>
/**
 * @brief 解析上位机发送的单个LORA AT指令JSON
 * @param recv_msg 接收到的JSON消息
 * @return AT指令字符串，需要调用者释放内存，NULL表示解析失败
 */
char *ProtocolManager::parse_host_at_command(const char *recv_msg)
{
    // 参数检查
    if (!recv_msg) 
    {
        printf("invalid input parameters\r\n");
        return NULL;
    }

    // 解析JSON根节点
    cJSON* json_root = cJSON_Parse(recv_msg);
    if(!json_root) 
    {
        printf("can't parse json_root is nullptr\r\n");
        return NULL;
    }

    // 获取request字段，检查是否为"lora"
    cJSON* request = cJSON_GetObjectItem(json_root, "request");
    if(!request || !cJSON_IsString(request) || !strstr(request->valuestring, "lora")) 
    {
        printf("request is not lora command\r\n");
        cJSON_Delete(json_root);
        return NULL;
    }

    // 获取param_list
    cJSON* param_list = cJSON_GetObjectItem(json_root, "param_list");
    if(!param_list || !cJSON_IsArray(param_list)) 
    {
        printf("can't parse param_list is nullptr or not array\r\n");
        cJSON_Delete(json_root);
        return NULL;
    }

    // 检查数组是否为空
    int param_count = cJSON_GetArraySize(param_list);
    if(param_count == 0) 
    {
        printf("param_list is empty\r\n");
        cJSON_Delete(json_root);
        return NULL;
    }

    // 获取第一个AT指令
    cJSON* item = cJSON_GetArrayItem(param_list, 0);
    if(!item || !cJSON_IsString(item) || !item->valuestring) 
    {
        printf("no valid AT command found\r\n");
        cJSON_Delete(json_root);
        return NULL;
    }

    char* at_command = (char*)malloc(strlen(item->valuestring) + 3);
    if(!at_command) 
    {
        printf("malloc for at_command failed\r\n");
        cJSON_Delete(json_root);
        return NULL;
    }

    // 处理+++特殊命令
    if(strstr(item->valuestring, "+++") != NULL) {
        // 移除所有\r\n字符
        char *src = item->valuestring;
        char *dst = at_command;
        while(*src) {
            if(*src != '\r' && *src != '\n') *dst++ = *src;
            src++;
        }
        *dst = '\0';
        printf("AT command (+++): %s\n", at_command);
    } 
    else {
        strcpy(at_command, item->valuestring);
        
        // 检查是否需要添加\r\n
        strcat(at_command, "\r\n");
        
        printf("AT command: %s", at_command);
    }

    cJSON_Delete(json_root);
    return at_command;
}

/**
 * @brief 包装LORA模块返回的结果为JSON格式
 * @param msg_id 消息ID
 * @param lora_response LORA模块返回的原始结果
 * @return JSON格式的字符串，需要调用者释放内存
 */
char *ProtocolManager::wrap_lora_response_json(const char* msg_id, const char* lora_response)
{
    // printf("start to wrap lora response json\r\n");
    cJSON* root = cJSON_CreateObject();
    
    // 添加msg_id
    if(msg_id) 
    {
        cJSON_AddStringToObject(root, "msg_id", msg_id);
    }
    else
    {
        cJSON_AddStringToObject(root, "msg_id", "");
    }
    
    // 添加device_id
    cJSON_AddNumberToObject(root, "device_id", get_history_config()->device_id);
    
    // 添加LORA返回结果
    if(lora_response)
    {
        cJSON_AddStringToObject(root, "result", lora_response);
    }
    else
    {
        cJSON_AddStringToObject(root, "result", "");
    }
        // 添加debug信息
    const char*debug_log = log_buffer_;
    if(strlen(debug_log) > 0)
    {
        cJSON_AddStringToObject(root,"debug",debug_log);
        internal_printf("Added debug log to JSON, length: %d\r\n", strlen(debug_log));
    }

    char* result_json = cJSON_Print(root);
    cJSON_Delete(root);
    
    return result_json;
}

void internal_printf(const char *format, ...)
{
     // 如果缓冲区快满了，清空一部分
    if (log_buffer_pos_ >= LOG_BUFFER_SIZE - 256) {
        clear_log();
    }
    
    va_list args;
    va_start(args, format);
    
    // 格式化输出到缓冲区
    int remaining_size = LOG_BUFFER_SIZE - log_buffer_pos_;
    if (remaining_size > 0) {
        int written = vsnprintf(log_buffer_ + log_buffer_pos_, remaining_size, format, args);
        if (written > 0) {
            log_buffer_pos_ += written;
            // 确保不超过缓冲区大小
            if (log_buffer_pos_ >= LOG_BUFFER_SIZE) {
                log_buffer_pos_ = LOG_BUFFER_SIZE - 1;
                log_buffer_[log_buffer_pos_] = '\0';
            }
        }
    }
    
    va_end(args);
    
    // // 同时输出到标准输出（可选，用于调试）
    // va_start(args, format);
    // vprintf(format, args);
    // va_end(args);
}

void clear_log()
{
    memset(log_buffer_, 0, LOG_BUFFER_SIZE);
    log_buffer_pos_ = 0;
}
