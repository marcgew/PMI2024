#ifndef SPI_H
#define SPi_H

/**
 * @brief GPIO & SPI init for communication with ADXL345
 */
int32_t spi_init_adxl345(void);

/**
 * @brief Transmit data via SPI
 * @param *buf uint8_t buffer to transmit
 * @param size Buffer size
 */
int32_t spi_txrx(uint8_t *buf, uint32_t size);

#endif /* SPI_H */