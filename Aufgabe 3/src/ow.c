#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ow.h>





void ow_init(void)
{


    RCC->IOPENR |= RCC_IOPENR_IOPBEN; // Enable GPIO clock for port B

    GPIOB->MODER &= ~(GPIO_MODER_MODE13_1); // Set pin B13 as output
    GPIOB->MODER |= GPIO_MODER_MODE13_0;

    GPIOB->OTYPER |= GPIO_OTYPER_OT_13; // Open-drain configuration

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD13_1); // Enable pull-up resistors
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD13_0;
}

/**
 * @brief Function to initialize Timer 2
 */
void ow_tim_init(void)
{
    // Reset TIM2 peripheral
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;

    // Enable the TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Configure TIM2
    TIM2->PSC = 16;      // Set prescaler for 1 kHz timer clock
    TIM2->DIER |= TIM_DIER_UIE; // Enable update interrupt


}

void ow_timer(uint16_t time)
{   
    TIM2->SR &= ~TIM_SR_UIF; // Clear the interrupt flag
    TIM2->CR1 &= ~ TIM_CR1_CEN; //disable timer

    TIM2->ARR = time;    

    TIM2->CR1 |= TIM_CR1_CEN; //enable timer 
    while(!(TIM2->SR & TIM_SR_UIF)) // wait for interrupt flag 
    {
       
    }
}

void ow_reset(void)
{
    pull_down;
    ow_timer(480);
    pull_up;
    ow_timer(240);
}

void ow_send_byte(uint8_t buffer)
{
    for (uint8_t i = 0; i < 8; i++)
    {

        if (buffer & 0x80) // masks 8 bit
        {
            pull_down;
            ow_timer(5);
            pull_up;
            ow_timer(55);
        }
        else
        {
            pull_down;
            ow_timer(60);
            pull_up;
        }

        buffer <<= 1;
        ow_timer(10);// delay (test) for next bit
    }
    ow_timer(100);
}


uint8_t ow_read_byte(void)
{
    uint8_t buffer = 0;

    for (uint8_t i = 0; i < 8; i++)
    {

        pull_down;
        // delay 15us <- read anfordern
        pull_up;
        // delay 20us <- Abtastung von IDR
        if ((GPIOB->IDR & GPIO_IDR_ID13))
        {
            buffer |= 1;
        }
        buffer <<= 1;
        //delay 10us <- read cycle completion 
    }
}


void ow_strong_pullup(void)
{



}
