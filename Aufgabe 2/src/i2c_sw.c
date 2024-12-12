#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <systick.h>
#include <i2c_sw.h>
#include <stdint.h>
#include <stddef.h>
#include <pmi_stddefs.h>



void i2c_sw_init(void)
{

    /* Enable GPIO clock for port B */
    RCC->IOPENR |= RCC_IOPENR_IOPBEN;

    GPIOB->MODER |= GPIO_MODER_MODE8_0; // set PA12 Output
    GPIOB->MODER &= ~(GPIO_MODER_MODE8_1);
    GPIOB->MODER |= GPIO_MODER_MODE9_0; // set PA12 Output
    GPIOB->MODER &= ~(GPIO_MODER_MODE9_1);

    /* Set output type to open drain for PB8, PB9 */
    GPIOB->OTYPER |= (GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);

    /* Set speed mode to medium speed */
    GPIOB->OSPEEDR |= GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0;



    // eventuell noch das anstelle vom high ziehen oben?
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD9_0;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD9_1);

    GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0;
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8_1);
}

void delay_func(void)
{
    for (uint16_t i = 0; i < 8000; i++)
    {
        
    }
}

void i2c_start_communication(void)
{

    SCL_HIGH;
    SDA_HIGH;
    delay_func();
    SDA_LOW;
    delay_func();
}

void i2c_stop_communication(void)
{
    SCL_LOW;
    SDA_LOW;
    delay_func();
    SCL_HIGH;
    delay_func();
    SDA_HIGH;
}

int32_t i2c_send_byte(uint8_t buffer)
{

    for (uint8_t i = 0; i < 8; i++)
    {
        SCL_LOW;
        delay_func();
        if (buffer & 0x80)
        {
            SDA_HIGH;
            buffer <<= 1;
        }
        else
        {
            SDA_LOW;
            buffer <<= 1;
        }
        delay_func();
        
        SCL_HIGH;
        delay_func();
    }
    SCL_LOW;
    delay_func();
    return RC_SUCC;
}

int32_t i2c_recieve_byte(uint8_t ack_bit)
{
    uint8_t Buffer = 0;

    // input mode 
    GPIOB->MODER &= ~GPIO_MODER_MODE9;      
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD9_1;      


    // for loop receive

    for (uint8_t i = 0; i < 8; i++)
    {
        SCL_LOW;
        delay_func();

        if ((GPIOB->IDR & GPIO_IDR_ID9)) //check state of input pin PB9
        {
        Buffer |= 1;
        }

        Buffer <<= 1;

        SCL_HIGH;
        delay_func();
    }

    // output mode
    GPIOB->MODER |= GPIO_MODER_MODE9_0; // set PB9 Output
    GPIOB->MODER &= ~(GPIO_MODER_MODE9_1);
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD9_0;      
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD9_1);

    if (ack_bit == 1)
    {
        SDA_LOW;
    }
    else
    {
        SDA_HIGH;
    }
    
    SCL_HIGH;
    delay_func();

    return Buffer;
}
