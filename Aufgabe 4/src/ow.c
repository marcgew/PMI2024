#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ow.h>

/**
 * @brief Initializes the OneWire interface by configuring GPIO settings
 */
void ow_init(void)
{

    RCC->IOPENR |= RCC_IOPENR_IOPBEN; // Enable GPIO clock for port B

    GPIOB->MODER &= ~(GPIO_MODER_MODE13_1); // Set pin B13 as output
    GPIOB->MODER |= GPIO_MODER_MODE13_0;

    GPIOB->OTYPER |= GPIO_OTYPER_OT_13; // Configure pin B13 as open-drain

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD13_1); // Enable pull-up resistors on pin B13
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD13_0;
}

/**
 * @brief Function to initialize Timer 2 for precise timing
 */
void ow_tim_init(void)
{
    // Reset TIM2 peripheral
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;

    // Enable the TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = 16; // Set prescaler for 1 kHz timer clock

    TIM2->DIER |= TIM_DIER_UIE; // Enable update interrupt
}

/**
 * @brief Delays the program for a specified time using Timer 2
 *
 * @param time Delay time in microseconds
 */
void ow_timer(uint16_t time)
{
    TIM2->SR &= ~TIM_SR_UIF; // Clear the interrupt flag

    TIM2->ARR = time; // delay time

    TIM2->CR1 |= TIM_CR1_CEN;        // enable timer
    while (!(TIM2->SR & TIM_SR_UIF)) // wait for interrupt flag
    {
    }
    TIM2->CR1 &= ~TIM_CR1_CEN; // disable timer
}

/**
 * @brief Enables strong pull-up mode for the OneWire data line
 */
void ow_strong_pullup(void)
{
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT_13; // Set to push-pull mode
    pull_up;
}

/**
 * @brief Disables the strong pull-up mode and sets back to open-drain mode
 */
void ow_disable_pullup(void)
{
    GPIOB->OTYPER |= GPIO_OTYPER_OT_13; // Set back to open-drain mode
}

/**
 * @brief Sends a reset pulse on the OneWire bus
 */
void ow_reset(void)
{
    pull_down;
    ow_timer(480);
    pull_up;
    ow_timer(240);
}

/**
 * @brief Writes a single bit to the OneWire bus
 *
 * @param bit The bit value to write 0 or 1
 */
void ow_write_bit(uint8_t bit)
{
    if (bit)
    {
        pull_down;
        ow_timer(5); // Write 1 Pull low for 5 µs
        pull_up;
        ow_timer(55); // Release for remainder of time slot
    }
    else
    {
        pull_down;
        ow_timer(60); // Write 0 Pull low for 60 µs
        pull_up;
    }
    ow_timer(10); // Small delay before next operation
}

/**
 * @brief Sends a byte to the OneWire bus
 *
 * @param buffer The byte to send
 */
void ow_send_byte(uint8_t buffer)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        ow_write_bit(buffer & 0x01); // Write LSB first
        buffer >>= 1;
    }
}

/**
 * @brief Reads a single bit from the OneWire bus
 *
 * @return The bit value
 */
uint8_t ow_read_bit(void)
{
    uint8_t bit;

    pull_down;
    ow_timer(2); // Initiate read slot Pull low for 2 µs
    pull_up;
    ow_timer(13); // Wait 13 µs before reading

    if ((GPIOB->IDR & GPIO_IDR_ID13)) // Read bit value
    {
        bit = 1;
    }
    else
    {
        bit = 0;
    }
    ow_timer(45); // Wait for remainder of time slot

    return bit;
}
/**
 * @brief Reads a byte from the OneWire bus
 *
 * @return The byte value
 */
uint8_t ow_read_byte(void)
{
    uint8_t byte = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        byte >>= 1;
        if (ow_read_bit())
        {
            byte |= 0x80; // Set MSB if bit is 1
        }
    }

    return byte;
}

/**
 * @brief Reads a sequence of bytes from the OneWire bus
 *
 * @param buf Pointer to the buffer where data will be stored
 * @param size Number of bytes to read
 */
void ow_read_buffer(uint8_t *buf, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        *buf = ow_read_byte(); // Read a byte and store it in the buffer
        buf++;                 // Move to the next buffer location
    }
}
