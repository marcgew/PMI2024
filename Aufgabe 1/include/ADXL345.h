#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

#define ADXL345_POWER_CTL 0x2D; // Power controll register
#define MSR_BIT 0b00001000;     // -> Enable measurement mode (bit 3)
#define MSR_ADR 0b11110010;     // 8-bit SPI address for reading data Bit 7 = 1 (Read), Bit 6 = 1 (Multi-byte), Bits 5-0 = 0x32 (DATA_X0 register)

void ADXL345_measure_init(void);
void ADXL345_collect_data(float *x_data, float *y_data, float *z_data);
uint8_t adxl_get_deviceid(void);

#endif /*ADXL345_H*/