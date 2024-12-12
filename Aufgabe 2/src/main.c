#include <uart.h>
#include <clocks.h>
#include <spi.h>
#include <ADXL345.h>
#include <systick.h>
#include <ili9341.h>
#include <pmi_string.h>
#include <i2c_sw.h>

int main(void)
{
    //Call your initialisations here
    clocks_init_pmi();
    uart_init_nucusb(115200);
    ili9341_init(0);
    i2c_sw_init();
  /*
    float x_float = 0;
    float y_float = 0;
    float z_float = 0;
    char x_str[7];
    char y_str[7];
    char z_str[7];
    ili9341_text_pos_set(1, 2);
    ili9341_str_print("ADXL345-X:", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 5);
    ili9341_str_print("ADXL345-Y:", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 8);
    ili9341_str_print("ADXL345-Z:", ILI9341_COLOR_DARKCYAN, ILI9341_COLOR_BLACK);

     */

    i2c_start_communication();
  
    i2c_send_byte(0b00011011);       //device address
      
    
    i2c_send_byte(0x0B);        //define set register
       

    i2c_send_byte(0b00000001);        //define set bit 
 

    i2c_send_byte(0b00001001);        //00001001 set continuous measurement mode
      


     i2c_send_byte(0b00011101);        //00011101 set continuous measurement bit

     i2c_send_byte(0b00000000);

     i2c_stop_communication();

    while (1)
    {
       
      //uint8_t blue = i2c_recieve_byte(1);
     // i2c_stop_communication();
        

        /*
        ADXL345_collect_data(&x_float, &y_float, &z_float);

        pmi_string_float2str(x_str, 7, x_float, 7);
        pmi_string_float2str(y_str, 7, y_float, 7);
        pmi_string_float2str(z_str, 7, z_float, 7);

        ili9341_text_pos_set(1, 3);
        ili9341_str_clear(10,ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(1, 3);
        ili9341_str_print(x_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
        uart_tx_str("x=");
        uart_tx_str(x_str);
        uart_tx_str("\n");

        ili9341_text_pos_set(1, 6);
        ili9341_str_clear(10,ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(1, 6);
        ili9341_str_print(y_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
        uart_tx_str("y=");
        uart_tx_str(y_str);
        uart_tx_str("\n");

        ili9341_text_pos_set(1, 9);
        ili9341_str_clear(10,ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(1, 9);
        ili9341_str_print(z_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
        uart_tx_str("z=");
        uart_tx_str(z_str);
        uart_tx_str("\n");

        systick_delay_ms(100);
        */
    }
}
