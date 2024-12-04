#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

/**
 * @brief 
*/
void ADXL345_measure_init(void);
void ADXL345_collect_data(float *x_data ,float *y_data, float *z_data);
uint8_t adxl_get_deviceid(void);

#endif /*ADXL345_H*/