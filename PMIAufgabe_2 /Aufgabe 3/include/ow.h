#ifndef OW_H
#define OW_H

 #define pull_down GPIOB->ODR &= ~(GPIO_ODR_OD13) // set ODR low to pull down OW line
 #define pull_up   GPIOB->ODR |= GPIO_ODR_OD13  // set ODR high to pull up OW line


/**
 * @brief Initializes the OneWire interface by configuring GPIO settings
 */
void ow_init(void);

/**
 * @brief Function to initialize Timer 2 for precise timing
 */
void ow_tim_init(void);

/**
 * @brief Delays the program for a specified time using Timer 2
 * 
 * @param time Delay time in microseconds
 */
void ow_timer(uint16_t time);

/**
 * @brief Enables strong pull-up mode for the OneWire data line
 */
void ow_strong_pullup(void);

/**
 * @brief Disables the strong pull-up mode and sets back to open-drain mode
 */
void ow_disable_pullup(void);

/**
 * @brief Sends a reset pulse on the OneWire bus
 */
void ow_reset(void);

/**
 * @brief Writes a single bit to the OneWire bus
 * 
 * @param bit The bit value to write 0 or 1
 */
void ow_write_bit(uint8_t bit);

/**
 * @brief Sends a byte to the OneWire bus
 * 
 * @param buffer The byte to send
 */
void ow_send_byte(uint8_t buffer);

/**
 * @brief Reads a single bit from the OneWire bus
 * 
 * @return The bit value 
 */
uint8_t ow_read_bit(void);

/**
 * @brief Reads a byte from the OneWire bus
 * 
 * @return The byte value
 */
uint8_t ow_read_byte(void);

/**
 * @brief Reads a sequence of bytes from the OneWire bus
 * 
 * @param buf Pointer to the buffer where data will be stored
 * @param size Number of bytes to read
 */
void ow_read_buffer(uint8_t *buf, uint8_t size);

#endif /* OW_H */