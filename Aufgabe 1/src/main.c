#include <uart.h>
#include <clocks.h>
#include <spi.h>



int main(void)
{
    /* Call your initialisations here */
    clocks_init_pmi();
    uart_init_nucusb(115200);
    spi_init_adxl345();

    int8_t buf[2]={0x2D,0x08};
    spi_txrx(buf,2);

    uint8_t values[] = {0xF2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    spi_txrx(values, 7);


    while(1)
    {   
       
        //spi_txrx(buf,10);

    }
}