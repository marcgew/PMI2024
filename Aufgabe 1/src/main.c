#include <uart.h>
#include <clocks.h>
#include <spi.h>
#include <ADXL345.h>
#include <systick.h>
#include <ili9341.h>
#include <pmi_string.h>

int main(void)
{
    clocks_init_pmi();
    uart_init_nucusb(115200);

    ili9341_init(0);
    ADXL345_measure_init();

    float x_float = 0;
    float y_float = 0;
    float z_float = 0;
    char x_str[10];
    char y_str[10];
    char z_str[10];

    ili9341_text_pos_set(1, 2);
    ili9341_str_print("ADXL345-X:", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 5);
    ili9341_str_print("ADXL345-Y:", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 8);
    ili9341_str_print("ADXL345-Z:", ILI9341_COLOR_DARKCYAN, ILI9341_COLOR_BLACK);

    while (1)
    {
    
    ADXL345_collect_data(&x_float, &y_float, &z_float);

        //Error-Mode if connection is lost
        if (x_float == 0.0f && y_float == 0.0f && z_float == 0.0f)
        {
            ili9341_text_pos_set(1, 11);
            ili9341_str_print("Sensor error, reinit", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);

            ADXL345_measure_init();
            systick_delay_ms(50);
            ili9341_text_pos_set(1, 11);
            ili9341_str_clear(16, ILI9341_COLOR_BLACK);
            ili9341_text_pos_set(0, 12);
            ili9341_str_clear(4, ILI9341_COLOR_BLACK);
            continue;               // restart the while-loop
        }
        
        pmi_string_float2str(x_str, 7, x_float, 7);
        pmi_string_float2str(y_str, 7, y_float, 7);
        pmi_string_float2str(z_str, 7, z_float, 7);

        ili9341_text_pos_set(1, 3);
        ili9341_str_clear(10, ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(1, 3);
        ili9341_str_print(x_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(9, 3);
        ili9341_str_print("m/s^2", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

        ili9341_text_pos_set(1, 6);
        ili9341_str_clear(10, ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(1, 6);
        ili9341_str_print(y_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(9, 6);
        ili9341_str_print("m/s^2", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

        ili9341_text_pos_set(1, 9);
        ili9341_str_clear(10, ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(1, 9);
        ili9341_str_print(z_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
        ili9341_text_pos_set(9, 9);
        ili9341_str_print("m/s^2", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

        systick_delay_ms(100);
    }
}
