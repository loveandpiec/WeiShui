#pragma once
#include <stdint.h>

struct OI_sf6_status
{
	uint8_t status:1;//Bit0：传感器状态
	uint8_t leak:1;//Bit1: 泄漏报警
	uint8_t liquefy:1;//Bit2: 液化报警
	uint8_t lock2_status:1;//Bit3: 闭锁2节点接线故障状态
	uint8_t lock1_status:1;//Bit4: 闭锁1节点接线故障状态
	uint8_t node_fault:1;//Bit5: 报警节点接线故障状态
	uint8_t lock2_action:1;//Bit6: 闭锁2信号节点动作状态
	uint8_t lock1_action:1;//Bit7: 闭锁1信号节点动作状态
	uint8_t node_action:1;//Bit8: 报警信号节点动作状态
	uint8_t over_press:1;//Bit9：超压报警。
	uint8_t :6;
};


class calculate
{
public:
    struct param
    {
        float Sht30Temperature;
        float Sht30Humidity;
        float Ms5803Temperature;
        float Ms5803Pressure;
        float SF6_percentage;
    };

    struct result
    {
        float Tempture; //
        float Point;    //
        float Press;    //
        float P20;      //
        float P20_SF6;
        float P20_N2;
        float TD0;      //
        float PPMS;     // 
        float density;  //
        int32_t Time_flag;
        float RH_f = 0;
        uint16_t uf_value;
        uint16_t om_value;
        OI_sf6_status sf6_status;
    };
    static void calculate_sf6(result& res,param& p);
};