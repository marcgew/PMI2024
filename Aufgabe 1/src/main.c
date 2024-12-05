#include <uart.h>
#include <clocks.h>
#include <spi.h>
#include <ADXL345.h>
#include <systick.h>
#include <ili9341.h>
#include <pmi_string.h>

int main(void)
{
    /* Call your initialisations here */
    clocks_init_pmi();
    uart_init_nucusb(115200);
    spi_init_adxl345();
    //ili9341_init(0);

    ADXL345_measure_init();
    float acc_x = 0x0000;
    float acc_y = 0x0000;
    float acc_z = 0x0000;
  

    float x_float = 0;
    float y_float = 0;
    float z_float = 0;
    char x_str[10]; 
    char y_str[10]; 
    char z_str[10]; 

    while(1)
    {   
        
        ADXL345_collect_data(&x_float, &y_float, &z_float);

        pmi_string_float2str(x_str, 10, x_float, 10);
        pmi_string_float2str(y_str, 10, y_float,10);
        pmi_string_float2str(z_str, 10, z_float, 10);


        uart_tx_str(x_str);
        uart_tx_str("\n");
        systick_delay_ms(1000);
        uart_tx_str(y_str);
        uart_tx_str("\n");
        systick_delay_ms(1000);
        uart_tx_str(z_str);
        uart_tx_str("\n");
        systick_delay_ms(1000);
        //ili9341_text_pos_set(10, 10);
        //ili9341_str_print(x_str[10], ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_WHITE);


        //ili9341_text_pos_set(20, 10);
        //ili9341_str_print(y_str[10], ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_WHITE);
        //systick_delay_ms(1000);

        //ili9341_text_pos_set(30, 10);
        //ili9341_str_print(z_str[10], ILI9341_COLOR_LIGHTGREY, ILI9341_COLOR_WHITE);
        //systick_delay_ms(1000);
           // adxl_get_deviceid();
         systick_delay_ms(3000);
        
    }
}
