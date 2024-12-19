#include <uart.h>
#include <stm32l0xx.h>
#include <stdarg.h>
#include <stdio.h>
#include <ow.h>
#include <DS18B20.h>


void DS18B20_config(void)
{

    ow_reset();
    //wait for presence pulse
    ow_send_byte(0xCC); //skip rom
    ow_send_byte(0x4E); // write Scratchpad 
    
    


}

void DS18B20_read(void)
{
    ow_reset();
    //wait for presence pulse
    ow_send_byte(0xCC); //skip rom
    ow_send_byte(0b01000100); 

   // ow_reset();
    //ow_send_byte(0xCC);
   // ow_send_byte(0xBE); // read Scratchpad 

    //Read 9Bits //

    //strong pullup 10ms 





}