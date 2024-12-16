#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ow.h>



void ow_init(void)
{

    RCC->IOPENR |= RCC_IOPENR_IOPBEN; // Enable GPIO clock for port B

    GPIOB->MODER &= ~(GPIO_MODER_MODE13_1); // Set pins as output
    GPIOB->MODER |= GPIO_MODER_MODE13_0;

    GPIOB->OTYPER |= GPIO_OTYPER_OT_13; // Open-drain configuration

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD13_1); // Enable pull-up resistors
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD13_0;
    
}

