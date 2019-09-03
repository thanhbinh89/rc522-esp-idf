#ifndef __SPI_H_
#define __SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//#define DRV_DEBUG    1

void spi_init(uint8_t clk, uint8_t mosi, uint8_t miso);
void spi_send(uint8_t *data, uint16_t len, uint8_t* cs);
void spi_receive(uint8_t *txbuff, uint16_t txlen, uint8_t *rxbuff, uint16_t rxlen, uint8_t* cs);
uint8_t spi_xchg(uint8_t data_send);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*TP_SPI_H*/