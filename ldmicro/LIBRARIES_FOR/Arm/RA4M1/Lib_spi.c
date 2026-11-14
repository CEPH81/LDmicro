/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_spi.h"

#define LIB_SPI_TIMEOUT 1000000U

#define SPSR_SPRF 0x80U  /* receive buffer full */
#define SPSR_SPTEF 0x20U /* transmit buffer empty */

uint16_t LibSPI_GetPrescaler(SPI_TypeDef *SPIx, uint32_t MAX_SPI_Frequency)
{
    uint32_t brdv, spbr;

    (void)SPIx;

    /* Bit rate = PCLKA / (2 * (SPBR + 1) * 2^BRDV) */
    if(MAX_SPI_Frequency == 0)
        return (uint16_t)((3U << 8) | 255U);
    for(brdv = 0; brdv < 4; brdv++) {
        uint32_t div = (2U << brdv) * MAX_SPI_Frequency;
        spbr = (LIB_PCLKA_HZ + div - 1U) / div; /* (SPBR + 1), rounded up: never faster than asked */
        if(spbr == 0)
            spbr = 1;
        if(spbr <= 256U)
            return (uint16_t)((brdv << 8) | (spbr - 1U));
    }
    return (uint16_t)((3U << 8) | 255U); /* slowest rate */
}

void LibSPI_Init(SPI_TypeDef *SPIx, LibSPI_DataSize_t SPI_DataSize, uint16_t SPI_BaudRatePrescaler)
{
    (void)SPI_DataSize;

    if(SPIx != R_SPI1)
        return;

    /* Module stop of SPI1: MSTPB18 */
    R_SYSTEM->PRCR = LIB_PRCR_UNLOCK;
    R_MSTP->MSTPCRB &= ~(1UL << 18);
    R_SYSTEM->PRCR = LIB_PRCR_LOCK;

    SPIx->SPCR = 0;                                    /* disabled */
    SPIx->SSLP = 0;
    SPIx->SPPCR = 0;
    SPIx->SPBR = (uint8_t)(SPI_BaudRatePrescaler & 0xFFU);
    SPIx->SPDCR = 0x40;                                /* SPBYT = 1: the data register is accessed in bytes */
    SPIx->SPCKD = 0;
    SPIx->SSLND = 0;
    SPIx->SPND = 0;
    SPIx->SPCR2 = 0x10;                                /* SCKASE: RSPCK stops rather than overrunning the receiver */
    /* CPHA = 0, CPOL = 0, BRDV, 8 bits (SPB = 0111b), MSB first */
    SPIx->SPCMD[0] = (uint16_t)((((uint32_t)SPI_BaudRatePrescaler >> 8) & 3U) << 2) | 0x0700U;

    /* RSPCKB = P111, MISOB = P110, MOSIB = P109 */
    LibGPIO_InitPeripheral(1, 11, LIB_PSEL_SPI);
    LibGPIO_InitPeripheral(1, 10, LIB_PSEL_SPI);
    LibGPIO_InitPeripheral(1, 9, LIB_PSEL_SPI);

    SPIx->SPCR = 0x08;  /* master, SPI operation, full duplex, no interrupt */
    (void)SPIx->SPCR;
    SPIx->SPCR = 0x48;  /* SPE = 1 */
}

uint16_t LibSPI_Send(SPI_TypeDef *SPIx, uint16_t data)
{
    uint32_t n;

    if((SPIx->SPCR & 0x40U) == 0)
        return 0;

    for(n = 0; ((SPIx->SPSR & SPSR_SPTEF) == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    SPIx->SPDR_BY = (uint8_t)data;

    for(n = 0; ((SPIx->SPSR & SPSR_SPRF) == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    if((SPIx->SPSR & SPSR_SPRF) == 0)
        return 0;

    return SPIx->SPDR_BY;
}

void LibSPI_WriteMulti(SPI_TypeDef *SPIx, uint8_t *dataOut, uint32_t count)
{
    while(count--)
        (void)LibSPI_Send(SPIx, *dataOut++);
}
