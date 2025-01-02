#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ow.h>
#include <DS18B20.h>
#include <systick.h>

/**
 * @brief Initializes the DS18B20 temperature sensor
 * This function initalizes the OneWire interface and prepares it for communication
 */
void DS18B20_init(void)
{

    ow_init();
    ow_tim_init();
}

/**
 * @brief Reads the temperature from the DS18B20 sensor and converts it to Kelvin.
 *
 * @return Temperature in Kelvin as a 32-bit unsigned integer.
 */
uint32_t DS18B20_get_temp(void)
{

    ow_reset();
    ow_send_byte(DS_SKIPROM);   // Skip ROM selection
    ow_send_byte(DS_CONVERT_T); // Send command to start temperature conversion
    ow_strong_pullup();         // Enable strong pull-up for sensor operation
    systick_delay_ms(750);      // wait for conversion of Temperature
    ow_disable_pullup();        // Disable strong pull-up

    // Read the temperature data from the DS18B20 scratchpad
    ow_reset();
    ow_send_byte(DS_SKIPROM);
    ow_send_byte(DS_READ_SCRATCHPAD);

    // Extract and process raw temperature data
    uint8_t read_data[9] = {0};
    ow_read_buffer(read_data, 9);
    int16_t raw_temp = (int16_t)(read_data[1] << 8 | read_data[0]); // Combine MSB and LSB
    uint32_t final_temp = ((float)raw_temp / 16.0) + 273.15;        // Temperature to Kelvin

    return final_temp; // Return the temperature in Kelvin
}