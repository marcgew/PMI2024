#include <uart.h>
#include <clocks.h>
#include <systick.h>
#include <ili9341.h>
#include <pmi_string.h>
#include <ow.h> 
#include <DS18B20.h>

int main(void)
{
  
  clocks_init_pmi();
  uart_init_nucusb(115200);
  ili9341_init(0);

  DS18B20_init();
 
  float x_float = 0;

  char x_str[4];
 
  
  ili9341_text_pos_set(1, 2);
  ili9341_str_print("DS18B20:", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);

  

  while (1)
  { 

    x_float = DS18B20_get_temp();


    

    pmi_string_float2str(x_str, 4, x_float, 4);
 

    ili9341_text_pos_set(1, 3);
    ili9341_str_clear(10, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 3);
    ili9341_str_print(x_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(4, 3);
    ili9341_str_print("K", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    
    systick_delay_ms(100);

    
  }
}
