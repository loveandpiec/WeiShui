#pragma once
#include <stm32l4xx.h>
#include <peripheral_register.h>
//#include <gpio.h>
void debug_led();
namespace modern_framework
{
    namespace mcu
    {
        namespace gpio
        {
            enum class mode
            {
                input = 0,
                output = 1,
                alternate_function = 2,
                analog = 3
            };

            enum class output_type
            {
                pushpull=0,
                open_drain=1
            };

            enum class output_speed
            {
                low=0,
                medium=1,
                high=2,
                very_high=3
            };

            enum class pull_type
            {
                no=0,
                up=1,
                down=2
            };

            enum class alternate
            {
                fn0=0,
                fn1=1,
                fn2=2,
                fn3=3,
                fn4=4,
                fn5=5,
                fn6=6,
                fn7=7,
                fn8=8,
                fn9=9,
                fn10=10,
                fn11=11,
                fn12=12,
                fn13=13,
                fn14=14,
                fn15=15
            };


            constexpr uint32_t pin0 = 1 << 0,
                               pin1 = 1 << 1,
                               pin2 = 1 << 2,
                               pin3 = 1 << 3,
                               pin4 = 1 << 4,
                               pin5 = 1 << 5,
                               pin6 = 1 << 6,
                               pin7 = 1 << 7,
                               pin8 = 1 << 8,
                               pin9 = 1 << 9,
                               pin10 = 1 << 10,
                               pin11 = 1 << 11,
                               pin12 = 1 << 12,
                               pin13 = 1 << 13,
                               pin14 = 1 << 14,
                               pin15 = 1 << 15;

            struct moder
            {
                mode pin0 : 2;
                mode pin1 : 2;
                mode pin2 : 2;
                mode pin3 : 2;
                mode pin4 : 2;
                mode pin5 : 2;
                mode pin6 : 2;
                mode pin7 : 2;
                mode pin8 : 2;
                mode pin9 : 2;
                mode pin10 : 2;
                mode pin11 : 2;
                mode pin12 : 2;
                mode pin13 : 2;
                mode pin14 : 2;
                mode pin15 : 2;
            };

            struct otyper
            {
                output_type pin0 : 1;
                output_type pin1 : 1;
                output_type pin2 : 1;
                output_type pin3 : 1;
                output_type pin4 : 1;
                output_type pin5 : 1;
                output_type pin6 : 1;
                output_type pin7 : 1;
                output_type pin8 : 1;
                output_type pin9 : 1;
                output_type pin10 : 1;
                output_type pin11 : 1;
                output_type pin12 : 1;
                output_type pin13 : 1;
                output_type pin14 : 1;
                output_type pin15 : 1;
                uint8_t : 8;
                uint8_t : 8;
            };

            struct ospeedr
            {
                
                output_speed pin0 : 2;
                output_speed pin1 : 2;
                output_speed pin2 : 2;
                output_speed pin3 : 2;
                output_speed pin4 : 2;
                output_speed pin5 : 2;
                output_speed pin6 : 2;
                output_speed pin7 : 2;
                output_speed pin8 : 2;
                output_speed pin9 : 2;
                output_speed pin10 : 2;
                output_speed pin11 : 2;
                output_speed pin12 : 2;
                output_speed pin13 : 2;
                output_speed pin14 : 2;
                output_speed pin15 : 2;
            };

            struct pupdr
            {
                pull_type pin0 :2;
                pull_type pin1 :2;
                pull_type pin2 :2;
                pull_type pin3 :2;
                pull_type pin4 :2;
                pull_type pin5 :2;
                pull_type pin6 :2;
                pull_type pin7 :2;
                pull_type pin8 :2;
                pull_type pin9 :2;
                pull_type pin10 :2;
                pull_type pin11 :2;
                pull_type pin12 :2;
                pull_type pin13 :2;
                pull_type pin14 :2;
                pull_type pin15 :2;
            };

            struct bsrr
            {
                bool pin0set : 1;
                bool pin1set : 1;
                bool pin2set : 1;
                bool pin3set : 1;
                bool pin4set : 1;
                bool pin5set : 1;
                bool pin6set : 1;
                bool pin7set : 1;
                bool pin8set : 1;
                bool pin9set : 1;
                bool pin10set : 1;
                bool pin11set : 1;
                bool pin12set : 1;
                bool pin13set : 1;
                bool pin14set : 1;
                bool pin15set : 1;
                bool pin0reset : 1;
                bool pin1reset : 1;
                bool pin2reset : 1;
                bool pin3reset : 1;
                bool pin4reset : 1;
                bool pin5reset : 1;
                bool pin6reset : 1;
                bool pin7reset : 1;
                bool pin8reset : 1;
                bool pin9reset : 1;
                bool pin10reset : 1;
                bool pin11reset : 1;
                bool pin12reset : 1;
                bool pin13reset : 1;
                bool pin14reset : 1;
                bool pin15reset : 1;
            };

            struct brr
            {
                bool pin0 : 1;
                bool pin1 : 1;
                bool pin2 : 1;
                bool pin3 : 1;
                bool pin4 : 1;
                bool pin5 : 1;
                bool pin6 : 1;
                bool pin7 : 1;
                bool pin8 : 1;
                bool pin9 : 1;
                bool pin10 : 1;
                bool pin11 : 1;
                bool pin12 : 1;
                bool pin13 : 1;
                bool pin14 : 1;
                bool pin15 : 1;
                uint8_t : 8;
                uint8_t : 8;
            };

            struct iodr
            {
                bool pin0 : 1;
                bool pin1 : 1;
                bool pin2 : 1;
                bool pin3 : 1;
                bool pin4 : 1;
                bool pin5 : 1;
                bool pin6 : 1;
                bool pin7 : 1;
                bool pin8 : 1;
                bool pin9 : 1;
                bool pin10 : 1;
                bool pin11 : 1;
                bool pin12 : 1;
                bool pin13 : 1;
                bool pin14 : 1;
                bool pin15 : 1;
                uint8_t : 8;
                uint8_t : 8;
            };

            struct lckr
            {
                bool pin0 : 1;
                bool pin1 : 1;
                bool pin2 : 1;
                bool pin3 : 1;
                bool pin4 : 1;
                bool pin5 : 1;
                bool pin6 : 1;
                bool pin7 : 1;
                bool pin8 : 1;
                bool pin9 : 1;
                bool pin10 : 1;
                bool pin11 : 1;
                bool pin12 : 1;
                bool pin13 : 1;
                bool pin14 : 1;
                bool pin15 : 1;
                bool lock_key : 1;
                uint8_t : 7;
                uint8_t : 8;
            };

            struct afrl
            {
                alternate pin0:4; 
                alternate pin1:4;
                alternate pin2:4; 
                alternate pin3:4;
                alternate pin4:4; 
                alternate pin5:4;
                alternate pin6:4; 
                alternate pin7:4;
            };


            struct afrh
            {
                alternate pin8:4; 
                alternate pin9:4;
                alternate pin10:4; 
                alternate pin11:4;
                alternate pin12:4; 
                alternate pin13:4;
                alternate pin14:4; 
                alternate pin15:4;
            };

            template <unsigned long GPIO_BASE_ADDR>
            class constexpr_bank
            {

                template <class T, typename M , M field,size_t index>
                struct register_helper
                {
                    using type = peripheral_reigster<T, mk_offset<GPIO_BASE_ADDR + get_offset<M, field,index>::offset>>;
                };
            public:
                template <class T, volatile uint32_t GPIO_TypeDef::*field>
                using gpio_register = typename register_helper<T,volatile uint32_t GPIO_TypeDef::* ,field,0>::type;

                template <class T, volatile uint32_t (GPIO_TypeDef::*field)[],size_t index>
                using gpio_registers = typename register_helper<T,volatile uint32_t (GPIO_TypeDef::*)[] ,field,index>::type;

                gpio_register<moder, &GPIO_TypeDef::MODER> mode_reg = {};
                gpio_register<otyper, &GPIO_TypeDef::OTYPER> output_type_reg = {};
                gpio_register<ospeedr, &GPIO_TypeDef::OSPEEDR> output_speed_reg = {};
                gpio_register<pupdr, &GPIO_TypeDef::PUPDR> pull_type_reg = {};
                gpio_register<iodr, &GPIO_TypeDef::IDR> input_data_reg = {};
                gpio_register<iodr, &GPIO_TypeDef::ODR> output_data_reg = {};
                gpio_register<bsrr, &GPIO_TypeDef::BSRR> bit_set_reset_reg = {};
                gpio_register<brr, &GPIO_TypeDef::BRR> bit_reset_reg = {};
                gpio_register<lckr, &GPIO_TypeDef::LCKR> lock_reg = {};
                gpio_registers<afrl,&GPIO_TypeDef::AFR,0> alternate_select_reg_0_7 = {};
                gpio_registers<afrh,&GPIO_TypeDef::AFR,1> alternate_select_reg_8_15 = {};

                using t=decltype(&GPIO_TypeDef::AFR);
                constexpr void setoutput(uint32_t pins) const
                {
                    bit_set_reset_reg = reinterpret_cast<const bsrr &>(pins);
                }
                constexpr void resetoutput(uint32_t pins) const
                {
                    bit_reset_reg = reinterpret_cast<const brr &>(pins);
                }
                constexpr uint32_t input_data() const
                {
                    return reinterpret_cast<uint32_t &>(static_cast<iodr &>(input_data_reg));
                }
                constexpr uint32_t output_data() const
                {
                    return reinterpret_cast<uint32_t &>(static_cast<iodr &>(output_data_reg));
                }

                constexpr constexpr_bank()
                {
                }
            };

            using bank_a = constexpr_bank<GPIOA_BASE>;
            constexpr bank_a pa;
            using bank_b = constexpr_bank<GPIOB_BASE>;
            constexpr bank_b pb;
            using bank_c = constexpr_bank<GPIOC_BASE>;
            constexpr bank_c pc;
            using bank_d = constexpr_bank<GPIOD_BASE>;
            constexpr bank_d pd;
            // using bank_e = constexpr_bank<GPIOE_BASE>;
            // constexpr bank_e pe;
            // using bank_f = constexpr_bank<GPIOF_BASE>;
            // constexpr bank_f pf;
            // using bank_g = constexpr_bank<GPIOG_BASE>;
            // constexpr bank_g pg;

            class pin
            {
                uint32_t _gpio_bank:32;
                uint8_t _pinx:4;
                
            public:
                pin(int bank_a_g,int32_t p0_15):_gpio_bank(bank_a_g),_pinx(p0_15)
                {  
                }
                void set_value(bool yes)
                {
                    if(yes)
                        get_register(_gpio_bank,&GPIO_TypeDef::BSRR).set<uint32_t>(1<<_pinx);
                    else
                        get_register(_gpio_bank,&GPIO_TypeDef::BRR).set<uint32_t>(1<<_pinx);
                }
                bool get_value()
                {
                    constexpr int input=0;
                    constexpr int output=1;
                    switch ((get_register(_gpio_bank,&GPIO_TypeDef::MODER).as<uint32_t>()&(3<<(2*_pinx)))>>(2*_pinx))
                    {
                    case input:
                        return get_register(_gpio_bank,&GPIO_TypeDef::IDR).as<uint32_t>()&(1<<_pinx);
                    case output:
                        return get_register(_gpio_bank,&GPIO_TypeDef::ODR).as<uint32_t>()&(1<<_pinx);
                    default:
                        return false;
                    }
                }
                pin& mode(gpio::mode m)
                {
                    get_register(_gpio_bank,&GPIO_TypeDef::MODER).modify(m,_pinx*2,2);
                    return *this;
                }
                pin& alternate(gpio::alternate fun)
                {
                    get_registers(_gpio_bank,&GPIO_TypeDef::AFR,_pinx/8).modify(fun,(_pinx%8)*4,4);
                    return *this;
                }

                pin& pull_type(gpio::pull_type pt)
                {
                    get_register(_gpio_bank,&GPIO_TypeDef::PUPDR).modify(pt,_pinx<<1,2);
                    return *this;
                }

                pin& output_type(gpio::output_type ot)
                {
                    get_register(_gpio_bank,&GPIO_TypeDef::OTYPER).modify(ot,_pinx<<1,2);
                    return *this;
                }

                pin& output_speed(gpio::output_speed os)
                {
                    get_register(_gpio_bank,&GPIO_TypeDef::OSPEEDR).modify(os,_pinx<<1,2);
                    return *this;
                }
            };
        } // namespace gpio
    }
}