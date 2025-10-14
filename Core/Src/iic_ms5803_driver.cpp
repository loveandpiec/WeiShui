#include <iic_ms5803_driver.h>
#include <stm32l4xx_ll_dma.h>
#include <stm32l4xx_ll_i2c.h>
#include <stm32l4xx_ll_usart.h>
#include <stm32l4xx_ll_utils.h>
#include <stdio.h>
#include <task_queue.h>
extern "C"
{
    extern void (*iic_dma_rx_complete)(void *);
    extern void (*iic_dma_tx_complete)(void *);
    extern void (*iic_it_rx_complete)(void *, uint8_t);
    extern void *iic_current_device;
    extern void (*iic_it_tx_complete)(void *);
    int _write(int, char *, int);
}


void iic_ms5803_driver::read_adc()
{
    common();
    _current_rx_count = 0;
    LL_I2C_HandleTransfer(I2C1, 0x77 << 1, LL_I2C_ADDRSLAVE_7BIT, 3, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
}

void iic_ms5803_driver::common()
{
    iic_current_device = this;
    iic_it_rx_complete = WRAP_FUN(iic_rx_complete_handle);
    iic_it_tx_complete = WRAP_FUN(iic_tx_complete_handle);
}
void iic_ms5803_driver::send_command_word()
{
    common();
    LL_I2C_HandleTransfer(I2C1, 0x77 << 1, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
}

iic_ms5803_driver::iic_ms5803_driver()
{
    ad_sample_complete.connect(WRAP(on_sample_completed,this));
    _send_buffer[PRESS] = MS5803_CMD_ADC_CONV + MS5803_CMD_ADC_D1 + MS5803_CMD_ADC_4096;
    _send_buffer[PRESS_ADC] = 0;
    _send_buffer[TEMP] = MS5803_CMD_ADC_CONV + MS5803_CMD_ADC_D2 + MS5803_CMD_ADC_4096;
    _send_buffer[TEMP_ADC] = 0;
}

void iic_ms5803_driver::read_caldata()
{
    // fprintf(stderr,__FUNCTION__);
    LL_I2C_DisableIT_RX(I2C1);
    LL_I2C_DisableIT_TX(I2C1);
    uint8_t send_buffer[6] = {0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC};
    auto sync_read = [this, &send_buffer](uint8_t i) -> uint16_t
    {
        LL_I2C_HandleTransfer(I2C1, 0x77 << 1, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
        LL_I2C_TransmitData8(I2C1, send_buffer[i]);
        LL_mDelay(1);
        LL_I2C_HandleTransfer(I2C1, 0x77 << 1, LL_I2C_ADDRSLAVE_7BIT, 2, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
        LL_mDelay(1);
        int retry_times = 100;
        while (!LL_I2C_IsActiveFlag_RXNE(I2C1))
            if (!--retry_times)
            {
                break;
            }
        uint16_t cali_para = LL_I2C_ReceiveData8(I2C1) << 8;
        LL_mDelay(1);
        retry_times = 100;
        while (!LL_I2C_IsActiveFlag_RXNE(I2C1))
            if (!--retry_times)
            {
                break;
            }
        cali_para |= LL_I2C_ReceiveData8(I2C1);
        return cali_para;
    };
    for (int i = 0; i < 6; i++)
    {
        cali_para[i+1] = sync_read(i);
        // printf("cali_para=%d\n",cali_para[i+1]);
        LL_mDelay(1);
    }
    //LL_I2C_ClearFlag_TXE(I2C1);
    //LL_I2C_ClearFlag_STOP(I2C1);
    NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C1_ER_IRQn);
    LL_I2C_EnableIT_RX(I2C1);
    LL_I2C_EnableIT_TX(I2C1);
}
void iic_ms5803_driver::read()
{
    _finish_tx = false;
    _finish_rx = false;
    _current_reading_task = PRESS;
    send_command_word();
    // fprintf(stderr,"%s\n","iic_ms5803_driver::read");
}

void iic_ms5803_driver::iic_rx_complete_handle(uint8_t ch)
{
    _buffer[_current_rx_count++] = ch;
    if (_current_rx_count == 3)
    {
        switch (_current_reading_task)
        {
        case PRESS_ADC:
            _current_reading_task = TEMP;
            pressure_ad = (_buffer[0] << 16) | (_buffer[1] << 8) | _buffer[2];
            _timer.executate_after(50, WRAP(send_command_word, this));
            break;
        case TEMP_ADC:
            temperature_ad = (_buffer[0] << 16) | (_buffer[1] << 8) | _buffer[2];
            ad_sample_complete();
            break;
        default:
            break;
        }
        _current_rx_count = 0;
    }
}

void iic_ms5803_driver::iic_tx_complete_handle()
{
    switch (_current_reading_task)
    {
    case PRESS:
        LL_I2C_TransmitData8(I2C1, _send_buffer[PRESS]);
        _current_reading_task = PRESS_ADC;
        _timer.executate_after(20, WRAP(send_command_word, this));
        break;
    case PRESS_ADC:
        LL_I2C_TransmitData8(I2C1, _send_buffer[PRESS_ADC]);
        _timer.executate_after(20, WRAP(read_adc, this));
        break;
    case TEMP:
        LL_I2C_TransmitData8(I2C1, _send_buffer[TEMP]);
        _current_reading_task = TEMP_ADC;
        _timer.executate_after(20, WRAP(send_command_word, this));
        break;
    case TEMP_ADC:
        LL_I2C_TransmitData8(I2C1, _send_buffer[TEMP_ADC]);
        _timer.executate_after(20, WRAP(read_adc, this));
        break;
    default:
        break;
    }
    _current_rx_count = 0;
}

// void iic_ms5803_driver::dma_rx_complete_handle()
// {
//     if (LL_DMA_IsActiveFlag_TC3(DMA1))
//     {
//         LL_DMA_ClearFlag_TC6(DMA1);
//         LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_7);
//         _finish_rx = true;
//     }
// }

template<uint32_t base,uint32_t m>
struct cal_pow2
{
    static constexpr int64_t value=cal_pow2<base,m-1>::value*base;
};

template<uint32_t base>
struct cal_pow2<base,0>
{
    static constexpr int64_t value=1;
};

template<uint32_t base,uint32_t m>
constexpr int64_t pow=cal_pow2<base,m>::value;

void iic_ms5803_driver::on_sample_completed()
{
    int32_t dT;
    int32_t temp;
    int32_t pres;

    int64_t off;
    int64_t sens;

    int64_t ti = 0;
    int64_t offi = 0;
    int64_t sensi = 0;

    // 对温度进行一阶修正
    dT = temperature_ad - cali_para[5] * 256;
    temp = (int32_t)(2000 + dT * cali_para[6] / pow<2, 23>);

    // 对压力进行一阶修正
    off = (int64_t)(cali_para[2] * pow<2, 16> + (cali_para[4] * dT) / pow<2, 7>);
    sens = (int64_t)(cali_para[1] * pow<2, 15> + (cali_para[3] * dT) / pow<2, 8>);
    pres = (int32_t)(((pressure_ad * sens) / pow<2, 21> - off) / pow<2, 15>);

    // 对压力温度进行二阶修正
    if (temp < 2000)
    {
        ti = (int64_t)(3 * dT * dT / pow<2, 33>);
        offi = (int64_t)(3 * (2000 - temp) * (2000 - temp) / 2);
        sensi = (int64_t)(5 * (2000 - temp) * (2000 - temp) / 8);

        if (temp < -1500)
        {
            offi = (int64_t)(offi + 7 * (1500 + temp) * (1500 + temp));
            sensi = (int64_t)(sensi + 4 * (1500 + temp) * (1500 + temp));
        }
    }
    else
    {
        ti = (int64_t)(7 * dT * dT / pow<2, 37>);
        offi = (int64_t)((temp - 2000) * (temp - 2000) / 16);
        sensi = 0;
    }

    int32_t Ms5803Temperature = temp - ti;
    off = off - offi;
    sens = sens - sensi;
    int32_t Ms5803Pressure = pres;
    sample_complete(Ms5803Temperature/100,Ms5803Pressure*10);
}