#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <spi.h>


/**
 * @brief Initialize the ADXL345 accelerometer for measurement mode
 */
void ADXL345_measure_init(void)
{

    // Initialize measurement mode for ADXL345
    int8_t init_buffer[2] = {ADXL345_POWER_CTL, MSR_BIT};
    spi_txrx(init_buffer, 2);
}

/**
 * @brief Collect acceleration data from ADXL345
 * @param[out] x_data Pointer to store acceleration data for the X-axis in m/s²
 * @param[out] y_data Pointer to store acceleration data for the Y-axis in m/s²
 * @param[out] z_data Pointer to store acceleration data for the Z-axis in m/s²
 */
void ADXL345_collect_data(float *x_data, float *y_data, float *z_data)
{

    uint8_t measure_values[7] = {MSR_ADR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    spi_txrx(measure_values, 7);

    // Combine LSB and MSB for each axis to form 16-bit values
    *x_data = ((measure_values[2] << 8) | measure_values[1]);
    *y_data = ((measure_values[4] << 8) | measure_values[3]);
    *z_data = ((measure_values[6] << 8) | measure_values[5]);

    // Convert raw data to acceleration values in m/s²
    // Scale factor: 0.0039g per LSB (datasheet), multiply by 9.81 to convert to m/s²
    *x_data = (float)((int16_t)(*x_data) * 0.0039) * 9.81;
    *y_data = (float)((int16_t)(*y_data) * 0.0039) * 9.81;
    *z_data = (float)((int16_t)(*z_data) * 0.0039) * 9.81;
}

/**
 * @brief Retrieve the device ID of the ADXL345
 * @return uint8_t The device ID value read from the ADXL345
 */
uint8_t adxl_get_deviceid(void)
{
    uint8_t buf[2] = {0b10000000, 0x00}; // 0b10000000 -> 8-bit SPI address for reading the DEVID register
    spi_txrx(buf, 2);
    return buf[1];
}