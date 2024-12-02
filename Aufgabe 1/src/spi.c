#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>

int32_t spi_init_adxl345(void)
{
    RCC->IOPENR |= RCC_IOPENR_IOPAEN;   //Enable GPIO clock for port A

    GPIOA->MODER &= ~(GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0);    //Reset mode for PA05, PA96, PA07

    GPIOA->MODER |= (GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);     //Set alternate function mode

    GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL5) | (GPIO_AFRL_AFSEL6) | (GPIO_AFRL_AFSEL7);      //Set alternate function 0 for PA05, PA06, PA07

    //SPI Chip Select auf PA12
    GPIOA->MODER |= GPIO_MODER_MODE12_0;
    GPIOA->MODER &= ~(GPIO_MODER_MODE12_1); 

    SPI_CR1_SPE;

    SPI1->CR1 |= SPI_CR1_BR;    
    SPI1->CR1 |= SPI_CR1_MSTR;  // Master Config
    SPI1->CR1 |= SPI_CR1_CPHA;  
    SPI1->CR1 |= SPI_CR1_CPOL;  
}

int32_t spi_txrx(uint8_t *buf, uint32_t size)
{
}