#ifndef display_control_H
#define display_control_H

#include <stdint.h>

/**
 * @brief Draws static text and labels on the display for sensor data visualization
 *        This function initializes the display and sets up fixed elements for the ADXL345, QMC5883L, and DS18B20
 */
void draw_solids(void);

/**
 * @brief Reads and displays data from the ADXL345 accelerometer on the screen
 */
void draw_ADXL345_Values(void);

/**
 * @brief Reads and displays data from the QMC5883L magnetometer on the screen
 */
void draw_QMC5883L_Values(void);

/**
 * @brief Reads and displays data from the DS18B20 temperature sensor on the screen
 */
void draw_DS18B20_Values(void);

/**
 * @brief Updates all sensor values on the display
 */
void draw_values(void);


#endif /* display_control_H */