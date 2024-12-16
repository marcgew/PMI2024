#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <systick.h>
#include <i2c_sw.h>
#include <QMC5883L.h>

/**
 * @brief Initializes the QMC5883L sensor with predefined settings
 *
 * This function configures the QMC5883L registers to enable measurement mode,
 * set the output data rate, and configure other necessary settings
 *
 */
int32_t qmc5883l_init(void)
{

    i2c_configure(QMC5883L_ADDR, QMC5883L_REG_SET_RESET, 0x01);      // enable reset behaviour
    i2c_configure(QMC5883L_ADDR, QMC5883L_REG_CONTROL1, 0x1D);       // continuous measurement mode
    i2c_configure(QMC5883L_ADDR, QMC5883L_REG_CONTROL2, 0b01000000); // default config

    return RC_SUCC;
}

/**
 * @brief Reads magnetic field data from the QMC5883L sensor.
 *
 * This function readsdata from the QMC5883L registers and converts it into values
 * representing the magnetic field strength in Tesla (T) for each axis
 *
 * @param[out] x Pointer to store the X-axis magnetic field strength in Tesla.
 * @param[out] y Pointer to store the Y-axis magnetic field strength in Tesla.
 * @param[out] z Pointer to store the Z-axis magnetic field strength in Tesla.
 *
 */
int32_t qmc5883l_mag_get(float *x, float *y, float *z)
{

    float temp_x = (i2c_recieve_data(QMC5883L_ADDR, X_MSB) << 8) | i2c_recieve_data(QMC5883L_ADDR, X_LSB);
    float temp_y = (i2c_recieve_data(QMC5883L_ADDR, Y_MSB) << 8) | i2c_recieve_data(QMC5883L_ADDR, Y_LSB);
    float temp_z = (i2c_recieve_data(QMC5883L_ADDR, Z_MSB) << 8) | i2c_recieve_data(QMC5883L_ADDR, Z_LSB);

    *x = temp_x / 10000000; // convert raw value (mG) to Tesla
    *y = temp_y / 10000000;
    *z = temp_z / 10000000;

    return RC_SUCC;
}