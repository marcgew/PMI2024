#include <uart.h>
#include <clocks.h>
#include <systick.h>
#include <ili9341.h>
#include <pmi_string.h>
#include <i2c_sw.h>
#include <QMC5883L.h>

int main(void)
{

  clocks_init_pmi();
  uart_init_nucusb(115200);
  ili9341_init(0);
  qmc5883l_init();

  float x_float = 0;
  float y_float = 0;
  float z_float = 0;
  char x_str[10];
  char y_str[10];
  char z_str[10];
  ili9341_text_pos_set(1, 2);
  ili9341_str_print("QMC5883L-X:", ILI9341_COLOR_RED, ILI9341_COLOR_BLACK);
  ili9341_text_pos_set(1, 5);
  ili9341_str_print("QMC5883L-Y:", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK);
  ili9341_text_pos_set(1, 8);
  ili9341_str_print("QMC5883L-Z:", ILI9341_COLOR_DARKCYAN, ILI9341_COLOR_BLACK);

  while (1)
  {

    qmc5883l_mag_get(&x_float, &y_float, &z_float);

    pmi_string_float2str(x_str, 10, x_float, 7);
    pmi_string_float2str(y_str, 10, y_float, 7);
    pmi_string_float2str(z_str, 10, z_float, 7);

    ili9341_text_pos_set(1, 3);
    ili9341_str_clear(10, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 3);
    ili9341_str_print(x_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(10, 3);
    ili9341_str_print("T", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(1, 6);
    ili9341_str_clear(10, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 6);
    ili9341_str_print(y_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(10, 6);
    ili9341_str_print("T", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    ili9341_text_pos_set(1, 9);
    ili9341_str_clear(10, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(1, 9);
    ili9341_str_print(z_str, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
    ili9341_text_pos_set(10, 9);
    ili9341_str_print("T", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

    systick_delay_ms(100);
  }
}
