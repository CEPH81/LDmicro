/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_SPI_H
#define LIB_SPI_H

/*
    Pins used (master, mode 0, MSB first, 8 bits):

    SPIx    SCK             MISO            MOSI
    SPI1    P111 (D13)      P110 (D12)      P109 (D11)

    The slave select is an ordinary output driven by the ladder program.
*/

#include "Lib_mcu.h"
#include "Lib_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LibSPI_DataSize_8b, /* SPI in 8 bits mode */
    LibSPI_DataSize_16b /* not supported: handled as 8 bits */
} LibSPI_DataSize_t;

/* Divider code giving the highest rate not above MAX_SPI_Frequency: SPBR in the low byte, BRDV in bits 8 and 9 */
uint16_t LibSPI_GetPrescaler(SPI_TypeDef *SPIx, uint32_t MAX_SPI_Frequency);

void     LibSPI_Init(SPI_TypeDef *SPIx, LibSPI_DataSize_t SPI_DataSize, uint16_t SPI_BaudRatePrescaler);
uint16_t LibSPI_Send(SPI_TypeDef *SPIx, uint16_t data);
void     LibSPI_WriteMulti(SPI_TypeDef *SPIx, uint8_t *dataOut, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif
