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
 * @brief Initializes software-based I2C by configuring GPIO pins
 *
 * Configures PB8 (SCL) and PB9 (SDA) as open-drain outputs with pull-up resistors
 */
void i2c_sw_init(void)
{
    RCC->IOPENR |= RCC_IOPENR_IOPBEN; // Enable GPIO clock for port B

    GPIOB->MODER &= ~(GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1); // Set pins as output
    GPIOB->MODER |= GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0;

    GPIOB->OTYPER |= GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9; // Open-drain configuration

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8_1 | GPIO_PUPDR_PUPD9_1); // Enable pull-up resistors
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0;
}
/**
 * @brief Generates a small delay for timing between I2C operations
 */
void delay_func(void)
{
    for (uint16_t i = 0; i < 100; i++)
    {
        __NOP();
    }
}

/**
 * @brief Initiates an I2C start condition
 *
 * Drives SDA low while SCL is high
 */
void i2c_start_communication(void)
{
    SCL_HIGH;
    SDA_HIGH;
    delay_func();
    SDA_LOW;
    delay_func();
    SCL_LOW;
    delay_func();
}

/**
 * @brief Initiates an I2C stop condition
 *
 * Drives SDA high while SCL is high
 */
void i2c_stop_communication(void)
{
    SCL_LOW;
    SDA_LOW;
    delay_func();
    SCL_HIGH;
    delay_func();
    SDA_HIGH;
}

/**
 * @brief Pulses the SCL line (SCL High -> SCL Low)
 */
void pulse_scl(void)
{
    SCL_HIGH;
    delay_func();

    SCL_LOW;
    delay_func();
}

/**
 * @brief Sends a byte of data via I2C
 *
 * @param buffer The byte to be sent
 */
int32_t i2c_send_byte(uint8_t buffer)
{

    for (uint8_t i = 0; i < 8; i++)
    {

        if (buffer & 0x80) // masks 8 bit
        {
            SDA_HIGH; // pull line up when bit is set
            buffer <<= 1;
        }
        else
        {
            SDA_LOW; // pull down when bit is 0
            buffer <<= 1;
        }

        delay_func();
        pulse_scl();
    }

    SDA_HIGH; // Release SDA for ACK
    delay_func();

    pulse_scl();

    int ack = !(GPIOB->IDR & GPIO_IDR_ID9); // Read ACK (low = ACK)

    if (ack)
    {
        return RC_SUCC;
    }
    else
    {
        return RC_ERR;
    }
}
/**
 * @brief Receives a byte of data via I2C
 *
 * Reads 8 bits of data sequentially from the SDA line, synchronized with the SCL clock
 *
 * @param ack_bit 1 to send an ACK, 0 to send a NACK after receiving the byte
 */
int32_t i2c_recieve_byte(uint8_t ack_bit)
{
    uint8_t buffer = 0;

    GPIOB->MODER &= ~GPIO_MODER_MODE9; // Configure SDA as input

    for (uint8_t i = 0; i < 8; i++)
    {

        SCL_HIGH;
        delay_func();

        buffer <<= 1;

        if ((GPIOB->IDR & GPIO_IDR_ID9))
        {
            buffer |= 1;
        }
        SCL_LOW;
        delay_func();
    }

    GPIOB->MODER |= GPIO_MODER_MODE9_0; //// Configure SDA as output

    if (ack_bit)
    {
        SDA_LOW; // Send ACK
    }
    else
    {
        SDA_HIGH; // Send NACK
    }

    pulse_scl();

    return buffer;
}

/**
 * @brief Configures a specific register on an I2C device
 *
 * Sends the device address, register address, and register value
 *
 * @param dev_add The I2C address of the device
 * @param reg_1 The address of the register to configure
 * @param reg_2 The value to write to the register
 */
void i2c_configure(uint8_t dev_add, uint8_t reg_1, uint8_t reg_2)
{
    i2c_start_communication();
    i2c_send_byte(dev_add);
    i2c_send_byte(reg_1);
    i2c_send_byte(reg_2);
    i2c_stop_communication();
}

/**
 * @brief Reads a single byte of data from a specific register on an I2C device
 *
 * Sends the device and register address, then reads the register's value
 *
 * @param dev_add The I2C address of the device
 * @param reg_add The address of the register to read
 */
uint8_t i2c_recieve_data(uint8_t dev_add, uint8_t reg_add)
{
    uint8_t BUF;
    i2c_start_communication();

    i2c_send_byte(dev_add); // Send device address (write)

    i2c_send_byte(reg_add); // Send register address

    i2c_stop_communication();

    i2c_start_communication(); // Repeated start for reading

    i2c_send_byte(dev_add | 0x01); // Send device address  1 for read

    BUF = i2c_recieve_byte(1); // Receive byte

    i2c_stop_communication();
    return BUF;
}