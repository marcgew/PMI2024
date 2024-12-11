#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <systick.h>
#include <i2c_sw.h>
#include <stdint.h>
#include <stddef.h>
#include <pmi_stddefs.h>
/**
 * This is a specific I2C software implementation for use with PMI-BoB2:
 * * SDA is on GPIOB pin 8 
 * * SCL is on GPIOB pin 9
 */

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

    // set SDA PB8 & SCL PB9 to high
    GPIOB->ODR |= GPIO_ODR_OD8;
    GPIOB->ODR |= GPIO_ODR_OD9;

    //eventuell noch das anstelle vom high ziehen oben?
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD9_0;      
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD9_1);
}

void i2c_start_communication(void)
{

    SCL_HIGH;
    SDA_HIGH;
    nop_50;// delay test
    SDA_LOW; //SDA_LOW;
    nop_50;
}

void i2c_stop_communication(void)
{
    SCL_LOW;
    SDA_LOW;
    nop_50; //delay
    SCL_HIGH;
    nop_50; //delay
    SDA_HIGH;

}


int32_t i2c_send(uint8_t buffer)
{
     if (buffer & 0x80)
     {
        SDA_HIGH;
     }
     else
    {
        SDA_LOW;
 
    }

     data <<=1; 


}

int32_t i2c_recieve(uint8_t ack)
{
    


}
