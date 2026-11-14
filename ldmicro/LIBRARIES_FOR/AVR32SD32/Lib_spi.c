/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_spi.h"

#define LIB_SPI_TIMEOUT 60000U

/* The rate is CLK_PER / 4, 16, 64 or 128, doubled with CLK2X */
uint16_t LibSPI_GetPrescaler(SPI_TypeDef *SPIx, uint32_t MAX_SPI_Frequency)
{
    static const uint8_t setting[7] = {
        SPI_CLK2X_bm | SPI_PRESC_DIV4_gc,  /* / 2 */
        SPI_PRESC_DIV4_gc,                 /* / 4 */
        SPI_CLK2X_bm | SPI_PRESC_DIV16_gc, /* / 8 */
        SPI_PRESC_DIV16_gc,                /* / 16 */
        SPI_CLK2X_bm | SPI_PRESC_DIV64_gc, /* / 32 */
        SPI_PRESC_DIV64_gc,                /* / 64 */
        SPI_PRESC_DIV128_gc,               /* / 128 */
    };
    uint8_t i;

    (void)SPIx;
    for(i = 0; i < 6; i++)
        if((SystemCoreClock >> (i + 1U)) <= MAX_SPI_Frequency)
            break;
    return setting[i];
}

void LibSPI_Init(SPI_TypeDef *SPIx, LibSPI_DataSize_t SPI_DataSize, uint16_t SPI_BaudRatePrescaler)
{
    (void)SPI_DataSize;

    if(SPIx != SPI0)
        return;

    SPIx->CTRLA = 0;

    /* default pins of SPI0: PA4 = MOSI, PA5 = MISO, PA6 = SCK */
    PORTMUX.SPIROUTEA = (uint8_t)((PORTMUX.SPIROUTEA & (uint8_t)~PORTMUX_SPI0_gm) | PORTMUX_SPI0_DEFAULT_gc);
    PORTA.OUTCLR = PIN4_bm | PIN6_bm;
    PORTA.DIRSET = PIN4_bm | PIN6_bm;
    PORTA.DIRCLR = PIN5_bm;

    SPIx->CTRLB = SPI_SSD_bm | SPI_MODE_0_gc; /* the SS pin is not used by the master */
    SPIx->CTRLA = (uint8_t)(SPI_MASTER_bm | (SPI_BaudRatePrescaler & (SPI_CLK2X_bm | SPI_PRESC_gm)) | SPI_ENABLE_bm);
}

uint16_t LibSPI_Send(SPI_TypeDef *SPIx, uint16_t data)
{
    uint16_t n;

    if((SPIx->CTRLA & SPI_ENABLE_bm) == 0)
        return 0;

    SPIx->DATA = (uint8_t)data;
    for(n = 0; ((SPIx->INTFLAGS & SPI_IF_bm) == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    return SPIx->DATA; /* reading the data after the flag clears the flag */
}

void LibSPI_WriteMulti(SPI_TypeDef *SPIx, uint8_t *dataOut, uint32_t count)
{
    while(count--)
        (void)LibSPI_Send(SPIx, *dataOut++);
}
