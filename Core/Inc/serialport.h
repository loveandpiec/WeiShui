#ifndef SERIALPORT_H
#define SERIALPORT_H
#include <stdint.h>
#include <stddef.h>
#include <stm32l412xx.h>

struct serial_gpio
{
    GPIO_TypeDef* gpio_bank;
    uint32_t gpio_pin;
    serial_gpio(GPIO_TypeDef* bank,uint32_t pin);
    serial_gpio(const serial_gpio& that);
    serial_gpio& operator=(const serial_gpio& that);
};

class serialport
{
protected:
    USART_TypeDef* _usart_handle;
    serial_gpio _tx_pin;
    serial_gpio _rx_pin;
public:
    void write(const void *buffer, size_t length);
    void serialport_it_rx_complete();
    virtual void on_recv(uint8_t ch);
    virtual ~serialport();
    serialport(USART_TypeDef* usart,const serial_gpio& tx,const serial_gpio& rx);
    virtual void open(int32_t baudrate,uint8_t databits,float stopbits,char parity);
};
#endif