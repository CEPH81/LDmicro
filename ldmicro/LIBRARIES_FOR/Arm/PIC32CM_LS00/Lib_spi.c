/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_spi.h"

#define LIB_SPI_TIMEOUT 100000U

#define SPI_DRE 0x01U
#define SPI_RXC 0x04U

static void LibSPI_Sync(SPI_TypeDef *SPIx)
{
    uint32_t n;

    for(n = 0; SPIx->SYNCBUSY && (n < LIB_SPI_TIMEOUT); n++)
        ;
}

/* rate = clock / (2 * (BAUD + 1)) */
uint16_t LibSPI_GetPrescaler(SPI_TypeDef *SPIx, uint32_t MAX_SPI_Frequency)
{
    uint32_t div;

    (void)SPIx;
    if(MAX_SPI_Frequency == 0)
        return 255;
    div = (SystemCoreClock + 2U * MAX_SPI_Frequency - 1U) / (2U * MAX_SPI_Frequency); /* rounded up */
    if(div < 1U)
        div = 1U;
    if(div > 256U)
        div = 256U;
    return (uint16_t)(div - 1U);
}

void LibSPI_Init(SPI_TypeDef *SPIx, LibSPI_DataSize_t SPI_DataSize, uint16_t SPI_BaudRatePrescaler)
{
    (void)SPI_DataSize;

    if(SPIx != SPI1)
        return;

    LibClock_Enable(LIB_GCLK_SERCOM0);

    SPIx->CTRLA = LIB_SERCOM_SWRST;
    LibSPI_Sync(SPIx);

    /* master, mode 0, MSB first; data out on PAD0, clock on PAD1, data in on PAD3 */
    SPIx->CTRLA = (3UL << 2) | (0UL << 16) | (3UL << 20);
    SPIx->BAUD8 = (uint8_t)SPI_BaudRatePrescaler;
    SPIx->CTRLB = (1UL << 17); /* receiver enabled, 8 bits */
    LibSPI_Sync(SPIx);

    /* PA04 = PAD0 (MOSI), PA05 = PAD1 (SCK), PA07 = PAD3 (MISO): function D */
    LibGPIO_InitPeripheral(0, 4, LIB_PMUX_D);
    LibGPIO_InitPeripheral(0, 5, LIB_PMUX_D);
    LibGPIO_InitPeripheral(0, 7, LIB_PMUX_D);

    SPIx->CTRLA |= LIB_SERCOM_ENABLE;
    LibSPI_Sync(SPIx);
}

uint16_t LibSPI_Send(SPI_TypeDef *SPIx, uint16_t data)
{
    uint32_t n;

    if((SPIx->CTRLA & LIB_SERCOM_ENABLE) == 0)
        return 0;

    for(n = 0; ((SPIx->INTFLAG & SPI_DRE) == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    SPIx->DATA = (uint8_t)data;
    for(n = 0; ((SPIx->INTFLAG & SPI_RXC) == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    return (uint16_t)(SPIx->DATA & 0xFFU);
}

void LibSPI_WriteMulti(SPI_TypeDef *SPIx, uint8_t *dataOut, uint32_t count)
{
    while(count--)
        (void)LibSPI_Send(SPIx, *dataOut++);
}
