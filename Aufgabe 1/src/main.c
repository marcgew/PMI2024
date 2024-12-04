#include <uart.h>
#include <clocks.h>
#include <spi.h>



int main(void)
{
    /* Call your initialisations here */
    clocks_init_pmi();
    uart_init_nucusb(115200);
    spi_init_adxl345();

    int8_t buf[2]={0x2D,0b00001000}; // 0x2D -> POWER_CTL REGISTER 0x08 -> Measure bit 3
    spi_txrx(buf,2);



    while(1)
    {   
          // 
          uint8_t values[] = {0xF2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 8 Bit Adresse bei SPI 6 Letzen bit sind Register das angesprochen wird(0x32) für DATA x 0 dann 8 und 7 Lese und Multibyte auf 1 setzen
          // sortierung nach X lsb X msb dann Y lsb MSB etc 
          spi_txrx(values, 7);

    }
}