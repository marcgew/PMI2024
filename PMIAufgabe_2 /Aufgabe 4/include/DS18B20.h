#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

#define DS_SKIPROM 0xCC //Command to skip ROM addressing
#define DS_CONVERT_T 0x44 //Command to start temperature conversion
#define DS_READ_SCRATCHPAD 0xBE //Command to read the scratchpad memory 

/**
 * @brief Initializes the DS18B20 temperature sensor
 * This function initalizes the OneWire interface and prepares it for communication
 */
void DS18B20_init(void);

/**
 * @brief Reads the temperature from the DS18B20 sensor and converts it to Kelvin.
 *
 * @return Temperature in Kelvin as a 32-bit unsigned integer.
 */
uint32_t DS18B20_get_temp(void);

#endif /*DS18B20_H*/