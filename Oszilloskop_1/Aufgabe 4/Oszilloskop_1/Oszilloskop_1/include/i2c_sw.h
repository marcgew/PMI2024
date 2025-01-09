#ifndef I2C_SW_H
#define I2C_SW_H

#include <stdint.h>

#define SCL_LOW GPIOB->ODR &= ~(GPIO_ODR_OD8) // Sets the SCL pin (PB8) to Low to disable the clock line
#define SCL_HIGH GPIOB->ODR |= GPIO_ODR_OD8   // Sets the SCL pin (PB8) to High to enable the clock line
#define SDA_LOW GPIOB->ODR &= ~(GPIO_ODR_OD9) // Sets the SDA pin (PB9) to Low to disable the data line
#define SDA_HIGH GPIOB->ODR |= GPIO_ODR_OD9   // Sets the SDA pin (PB9) to High to enable the data line

/**
 * @brief Initializes software-based I2C by configuring GPIO pins
 *
 * Configures PB8 (SCL) and PB9 (SDA) as open-drain outputs with pull-up resistors
 */
void i2c_sw_init(void);

/**
 * @brief Generates a small delay for timing between I2C operations
 */
void delay_func(void);

/**
 * @brief Initiates an I2C start condition
 *
 * Drives SDA low while SCL is high
 */
void i2c_start_communication(void);

/**
 * @brief Initiates an I2C stop condition
 *
 * Drives SDA high while SCL is high
 */
void i2c_stop_communication(void);

/**
 * @brief Pulses the SCL line (SCL High -> SCL Low)
 */
void pulse_scl(void);

/**
 * @brief Sends a byte of data via I2C
 *
 * @param buffer The byte to be sent
 */
int32_t i2c_send_byte(uint8_t buffer);

/**
 * @brief Receives a byte of data via I2C
 *
 * Reads 8 bits of data sequentially from the SDA line, synchronized with the SCL clock
 *
 * @param ack_bit 1 to send an ACK, 0 to send a NACK after receiving the byte
 */
int32_t i2c_recieve_byte(uint8_t ack_bit);

/**
 * @brief Configures a specific register on an I2C device
 *
 * Sends the device address, register address, and register value
 *
 * @param dev_add The I2C address of the device
 * @param reg_1 The address of the register to configure
 * @param reg_2 The value to write to the register
 */
void i2c_configure(uint8_t dev_add, uint8_t reg_1, uint8_t reg_2);

/**
 * @brief Reads a single byte of data from a specific register on an I2C device
 *
 * Sends the device and register address, then reads the register's value
 *
 * @param dev_add The I2C address of the device
 * @param reg_add The address of the register to read
 */
uint8_t i2c_recieve_data(uint8_t dev_add, uint8_t reg_add);

#endif /* I2C_SW_H */