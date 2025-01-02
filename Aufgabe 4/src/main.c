#include <uart.h>
#include <clocks.h>
#include <systick.h>
#include <pmi_string.h>
#include <ow.h>
#include <DS18B20.h>
#include <i2c_sw.h>
#include <QMC5883L.h>
#include <spi.h>
#include <ADXL345.h>
#include <ili9341.h>
#include <display_control.h>

int main(void)
{
  clocks_init_pmi();
  uart_init_nucusb(115200);

  // initialize all 3 Sensors 
  
  ADXL345_measure_init();
  qmc5883l_init();
  DS18B20_init();

  draw_solids(); // Draw static elements on the display (labels and units)

  while (1)
  {

    draw_values(); // Fetch sensor data and update their respective values on the display

    systick_delay_ms(100); // Add a delay of 100 milliseconds to control the update rate
  }
}
