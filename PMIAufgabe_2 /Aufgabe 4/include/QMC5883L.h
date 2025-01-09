#ifndef QMC5883L_H
#define QMC5883L_H

#include <stdint.h>

#define QMC5883L_ADDR 0x1A          // QMC Adress 0x0D << 1
#define QMC5883L_REG_CONTROL1 0x09  // Control Register 1
#define QMC5883L_REG_CONTROL2 0x0A  // Control Register 2
#define QMC5883L_REG_SET_RESET 0x0B // Set/Reset Period Register
#define X_LSB 0x00                  // X-axis LSB Register
#define X_MSB 0x01                  // X-axis MSB Register
#define Y_LSB 0x02                  // Y-axis LSB Register
#define Y_MSB 0x03                  // Y-axis MSB Register
#define Z_LSB 0x04                  // Z-axis LSB Register
#define Z_MSB 0x05                  // Z-axis MSB Register

/**
 * @brief Initializes the QMC5883L sensor with predefined settings
 *
 * This function configures the QMC5883L registers to enable measurement mode,
 * set the output data rate, and configure other necessary settings
 *
 */
int32_t qmc5883l_init(void);

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
int32_t qmc5883l_mag_get(float *x, float *y, float *z);

#endif /*QMC5883L.H*/