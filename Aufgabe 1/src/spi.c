#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>

int32_t spi_init_adxl345(void)
{
    RCC->IOPENR |= RCC_IOPENR_IOPAEN;   //Enable GPIO clock for port A
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    GPIOA->MODER &= ~(GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0);    //Reset mode for PA05, PA96, PA07

    GPIOA->MODER |= (GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);     //Set alternate function mode

    GPIOA->AFR[0] &= ~((GPIO_AFRL_AFSEL5) + (GPIO_AFRL_AFSEL6) + (GPIO_AFRL_AFSEL7));      //Set alternate function 0 for PA05, PA06, PA07
    //GPIOB->AFR[0] |= (5 << GPIO_AFRL_AFSEL5_Pos) | (5 << GPIO_AFRL_AFSEL6_Pos) | (5 << GPIO_AFRL_AFSEL7_Pos);


    //SPI Chip Select auf PA12  
    GPIOA->MODER |= GPIO_MODER_MODE12_0;
    GPIOA->MODER &= ~(GPIO_MODER_MODE12_1); 

    SPI1->CR1 &= ~SPI_CR1_SPE; //disable spi for config

    SPI1->CR1 &= ~SPI_CR1_BR_2;
    SPI1->CR1 |=  SPI_CR1_BR_0; // 
    SPI1->CR1 |=  SPI_CR1_BR_1;

    SPI1->CR1 |= SPI_CR1_MSTR;  // Master Config
    SPI1->CR1 |= SPI_CR1_CPHA;  
    SPI1->CR1 |= SPI_CR1_CPOL;  
    SPI1->CR1 |= SPI_CR1_SSM;   //Software Slave Management
    SPI1->CR1 |= SPI_CR1_SSI;   //Internal Slave Select        

    SPI1->CR1 |= SPI_CR1_SPE; //enable SPI 
} 

int32_t spi_txrx(uint8_t *buf, uint32_t size)
{
    GPIOA->ODR &= ~(GPIO_ODR_OD12);     //slave select

    for (uint8_t i = 0; i < size; i++)
    {
        while(!(SPI1->SR & SPI_SR_TXE))
        {
            //wait until transmit buffer is not empty
        }
        SPI1->DR = buf[i];     //transmit bit
        
        
        while(!(SPI1->SR & SPI_SR_RXNE))
        {
            //wait until receive buffer no longer empty and communication finished
        }
        buf[i] = SPI1->DR;      //read received bit
    }
    while (SPI1->SR & SPI_SR_BSY);
    {
        // wait until whole transfer is done 
    } 
    GPIOA->ODR |= GPIO_ODR_OD12; // unselect slave 

}

