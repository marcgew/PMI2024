#ifndef SPI_H
#define SPi_H


int32_t spi_init_adxl345(void);

int32_t spi_txrx(uint8_t *buf, uint32_t size);

#endif /* SPI_H */