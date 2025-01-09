#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <spi.h>

/**
 * @brief GPIO & SPI init for communication with ADXL345
 */
int32_t spi_init_adxl345(void)
{
    RCC->IOPENR |= RCC_IOPENR_IOPAEN; // Enable GPIO clock for port A
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    GPIOA->MODER &= ~(GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0); // Reset mode for PA05, PA96, PA07

    GPIOA->MODER |= (GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1); // Set alternate function mode

    // Set alternate function 0 for PA05, PA06, PA07
    GPIOA->AFR[0] &= ~((0xF << GPIO_AFRL_AFSEL5_Pos) | (0xF << GPIO_AFRL_AFSEL6_Pos) | (0xF << GPIO_AFRL_AFSEL7_Pos));

    // SPI Chip Select auf PA12
    GPIOA->MODER |= GPIO_MODER_MODE12_0; // set PA12 Output
    GPIOA->MODER &= ~(GPIO_MODER_MODE12_1);
    GPIOA->ODR |= GPIO_ODR_OD12; // set PA12 High (inital deselect)

    SPI1->CR1 &= ~(SPI_CR1_SPE); // disable spi for config

    SPI1->CR1 |= SPI_CR1_MSTR; // Master Config

    // Configure baud rate = / 32
    SPI1->CR1 |= SPI_CR1_BR_2;
    SPI1->CR1 &= ~SPI_CR1_BR_0;
    SPI1->CR1 &= ~SPI_CR1_BR_1;

    SPI1->CR1 |= SPI_CR1_CPHA; // set phase to 1
    SPI1->CR1 |= SPI_CR1_CPOL; // set polarity to 1
    SPI1->CR1 |= SPI_CR1_SSM;  // Software Slave Management
    SPI1->CR1 |= SPI_CR1_SSI;  // Internal Slave Select

    SPI1->CR1 |= SPI_CR1_SPE; // enable SPI

    return RC_SUCC;
}

/**
 * @brief Transmit data via SPI
 * @param *buf uint8_t buffer to transmit
 * @param size Buffer size
 */
int32_t spi_txrx(uint8_t *buf, uint32_t size)
{
    GPIOA->ODR &= ~(GPIO_ODR_OD12); // slave select

    for (uint8_t i = 0; i < size; i++)
    {
        while (!(SPI1->SR & SPI_SR_TXE))
        {
            // wait until transmit buffer is not empty
        }
        SPI1->DR = buf[i]; // transmit bit

        while (!(SPI1->SR & SPI_SR_RXNE))
        {
            // wait until receive buffer no longer empty and communication finished
        }
        buf[i] = SPI1->DR; // read received bit
    }

    GPIOA->ODR |= GPIO_ODR_OD12; // deselect slave

    return RC_SUCC;
}
