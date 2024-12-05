#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <spi.h>


void ADXL345_measure_init(void)
{

    int8_t init_buffer[2]={0x2D,0b00001000}; // 0x2D -> POWER_CTL REGISTER 0x08 -> Measure bit 3
    spi_txrx(init_buffer,2);

}

void ADXL345_collect_data(float *x_data , float *y_data, float *z_data)
{
     
    uint8_t measure_values[7] = {0b11110010, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 8 Bit Adresse bei SPI 6 Letzen bit sind Register das angesprochen wird(0x32) für DATA x 0 dann 8 und 7 Lese und Multibyte auf 1 setzen
    // sortierung nach X lsb X msb dann Y lsb MSB etc  0b11110010
    spi_txrx(measure_values, 7);

     *x_data =((measure_values[2]<< 8) | measure_values[1]);
     *y_data =((measure_values[4]<< 8) | measure_values[3]);
     *z_data =((measure_values[6]<< 8) | measure_values[5]);

     *x_data  = (float)((int16_t)(*x_data)* 0.0039)*9.81;
     *y_data  = (float)((int16_t)(*y_data)* 0.0039)*9.81;
     *z_data  = (float)((int16_t)(*z_data)* 0.0039)*9.81;
     
}

uint8_t adxl_get_deviceid(void)
{
    uint8_t buf[2]={0b10000000,0x00};
    spi_txrx(buf,2);
    return buf[1];
     
    
}