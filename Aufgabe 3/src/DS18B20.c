#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ow.h>
#include <DS18B20.h>
#include <systick.h>    

void DS18B20_init(void)
{
 
    ow_init();
    ow_tim_init();


}


uint32_t DS18B20_get_temp(void)
{
       
    ow_reset();
    ow_send_byte(DS_SKIPROM);
    ow_send_byte(DS_CONVERT_T); 
    ow_strong_pullup(); 
    systick_delay_ms(750); // wait for conversion of Temperature
    ow_disable_pullup();


    ow_reset();
    ow_send_byte(DS_SKIPROM);
    ow_send_byte(DS_READ_SCRATCHPAD);  

    uint8_t read_data[9] = {0};
    ow_read_buffer(read_data,9);
    int16_t raw_temp = (int16_t)(read_data[1] << 8 | read_data[0]);
    uint32_t final_temp = ((float)raw_temp / 16.0) + 273.15; // Temperature to Kelvin

    return final_temp;

}