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

    // Configure PB8 (SCL) and PB9 (SDA) as output, open-drain, pull-up
    GPIOB->MODER |= GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0;
    GPIOB->MODER &= ~(GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1);
    
    GPIOB->OSPEEDR |= GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0;

    GPIOB->OTYPER |= GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9; // Open-drain

    GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0; // Pull-up
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8_1 | GPIO_PUPDR_PUPD9_1);
    


    
}

void delay_func(void)
{
   for (uint16_t i = 0; i < 50; i++)
    {
        asm("nop");
     
    }
   //systick_delay_ms(1);
}

void i2c_start_communication(void)
{

    SCL_HIGH;
    delay_func();
    SDA_HIGH;
    delay_func();
    SDA_LOW;
    delay_func();
    SCL_LOW;
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

    for (uint8_t i = 8; i; i--)
    {
       
        
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

        SCL_LOW;
        delay_func();
    }

    // Check for ACK
    
    SDA_HIGH; // Release SDA for ACK
    delay_func();
    
    SCL_HIGH;
    delay_func();
    
    
    SCL_LOW;
    delay_func();

     int ack = !(GPIOB->IDR & GPIO_IDR_ID9); // Read ACK (low = ACK)

     return ack ? RC_SUCC : RC_ERR;
 
}

int32_t i2c_recieve_byte(uint8_t ack_bit)
{
    uint8_t buffer = 0;

    // Configure SDA as input
    GPIOB->MODER &= ~GPIO_MODER_MODE9;

    for (uint8_t i = 0; i < 8; i++) {
        SCL_LOW;
        delay_func();

        SCL_HIGH;
        delay_func();

        buffer <<= 1;
        if (GPIOB->IDR & GPIO_IDR_ID9) {
            buffer |= 1;
        }
    }

    SCL_LOW;

    // Configure SDA as output
    GPIOB->MODER |= GPIO_MODER_MODE9_0;

    if (ack_bit) {
        SDA_LOW; // Send ACK
    } else {
        SDA_HIGH; // Send NACK
    }

    delay_func();
    SCL_HIGH;
    delay_func();
    SCL_LOW;

    return buffer;
}

