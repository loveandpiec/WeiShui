#include <iic_sht35_driver.h>
#include <stm32l4xx_ll_dma.h>
#include <stm32l4xx_ll_i2c.h>
#include <stm32l4xx_ll_usart.h>
#include <stm32l4xx_ll_utils.h>
#include <stdio.h>

extern "C"
{
    extern void(*iic_it_rx_complete)(void*,uint8_t);
    extern void* iic_current_device;
    extern void(*iic_it_tx_complete)(void*);
    int _write(int,char* ,int);
}

iic_sht45_driver::iic_sht45_driver()
{
    _current_count=0;
    _send_buffer[0]=0x27;
    _send_buffer[1]=0x37;
    
}

void iic_sht45_driver::read()
{
    _finish_tx=false;
    _finish_rx=false;
    iic_current_device=this;
    iic_it_rx_complete=WRAP_FUN(iic_rx_complete_handle);
    iic_it_tx_complete=WRAP_FUN(iic_tx_complete_handle);

    LL_I2C_HandleTransfer(I2C1, 0x44<<1, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
    _current_count=0;
}

void iic_sht45_driver::read_value()
{
    iic_current_device=this;
    iic_it_rx_complete=[](void* pthis,uint8_t ch){reinterpret_cast<iic_sht45_driver*>(pthis)->iic_rx_complete_handle(ch);};
    iic_it_tx_complete=[](void* pthis){reinterpret_cast<iic_sht45_driver*>(pthis)->iic_tx_complete_handle();};
    LL_I2C_HandleTransfer(I2C1, 0x44<<1, LL_I2C_ADDRSLAVE_7BIT, 6, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
    _current_count=0;
}

void iic_sht45_driver::iic_rx_complete_handle(uint8_t ch)
{
    //fprintf(stderr,"%s\n",__FUNCTION__);
    _buffer[_current_count++]=ch;
    if(_current_count==6)
    {
        float Sht30Temperature = (175.0f*float((_buffer[0]<<8)+_buffer[1])/65535.0f-45.0f);
        float Sht30Humidity = (100.0f*float(_buffer[3]<<8)+float(_buffer[4]))/65535.0f;
        sample_ok(Sht30Temperature,Sht30Humidity);
        _current_count=0;
    }
}

void iic_sht45_driver::iic_tx_complete_handle()
{
    
    LL_I2C_TransmitData8(I2C1,0xFD);//_send_buffer[_current_count++]);
    //if(_current_count==1)
    {
        //_current_count=0;
        _timer.executate_after(100,WRAP(read_value,this));
    }
}
