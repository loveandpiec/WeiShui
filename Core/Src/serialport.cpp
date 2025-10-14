#include <serialport.h>
#include <math.h>
#include <stm32l4xx_ll_usart.h>
#include <stm32l4xx_ll_gpio.h>
#include <stm32l4xx_ll_utils.h>
#include <cstring>
#include <stdio.h>
serialport *lpuart1;
serialport *usart1;
serialport *usart2;
serialport *usart3;
serial_gpio::serial_gpio(GPIO_TypeDef *bank, uint32_t pin)
{
    gpio_bank = bank;
    gpio_pin = pin;
    
    
}

serial_gpio::serial_gpio(const serial_gpio &that)
{
    operator=(that);
}
serial_gpio &serial_gpio::operator=(const serial_gpio &that)
{
    gpio_bank = that.gpio_bank;
    gpio_pin = that.gpio_pin;
    return *this;
}

void serialport::write(const void *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        LL_USART_TransmitData8(_usart_handle, reinterpret_cast<const uint8_t *>(buffer)[i]);
        while (!LL_USART_IsActiveFlag_TC(_usart_handle));
    }
}
void serialport::serialport_it_rx_complete()
{
    if (LL_USART_IsActiveFlag_RXNE(_usart_handle))
    {
        LL_USART_ClearFlag_NE(_usart_handle);
        on_recv(LL_USART_ReceiveData8(_usart_handle));
    }
}
void serialport::on_recv(uint8_t ch)
{


}
serialport::~serialport() {};
serialport::serialport(USART_TypeDef *usart, const serial_gpio &gpio_tx, const serial_gpio &gpio_rx) : _tx_pin(gpio_tx), _rx_pin(gpio_rx)
{
    _usart_handle = usart;
    if (usart == USART1)
    {
        usart1 = this;

    }
        
    else if (usart == USART2)
    {
        usart2 = this;
    }
        
    else if (usart == USART3)
    {
        usart3 = this;
    }
    else if(usart == LPUART1)
    {
        lpuart1 = this;
    }
}

void serialport::open(int32_t baudrate, uint8_t databits, float stopbits, char parity)
{    
    LL_USART_InitTypeDef USART_InitStruct = {0};

    USART_InitStruct.BaudRate = baudrate;
    switch (databits)
    {
    case 7:
        USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_7B;
        break;
    case 9:
        USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_9B;
        break;
    default:
        USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
        break;
    }

    if(fabs(stopbits-0.5)<0.1)
        USART_InitStruct.StopBits = LL_USART_STOPBITS_0_5;
    else if(fabs(stopbits-1.5)<0.1)
        USART_InitStruct.StopBits = LL_USART_STOPBITS_1_5;
    else if(fabs(stopbits-2)<0.1)
        USART_InitStruct.StopBits = LL_USART_STOPBITS_2;
    else
        USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    
    switch (parity)
    {
    case 'E':
        USART_InitStruct.Parity = LL_USART_PARITY_EVEN;
        break;
    case 'O':
        USART_InitStruct.Parity = LL_USART_PARITY_ODD;
        break;
    default:
        USART_InitStruct.Parity = LL_USART_PARITY_NONE;
        break;
    }
    
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(_usart_handle, &USART_InitStruct);
    LL_USART_EnableOneBitSamp(_usart_handle);
    LL_USART_ConfigAsyncMode(_usart_handle);
    LL_USART_Enable(_usart_handle);
    LL_USART_EnableIT_RXNE(_usart_handle);
}
extern "C"
{
    void LPUART1_IRQHandler(void)
    {
        lpuart1->serialport_it_rx_complete();
    }

    void USART1_IRQHandler(void)
    {
        usart1->serialport_it_rx_complete();
    }

    void USART2_IRQHandler(void)
    {
        usart2->serialport_it_rx_complete();
    }
    /*
    void USART3_IRQHandler(void)
    {
        if(usart3)
            usart1->serialport_it_rx_complete();
    }*/
}
