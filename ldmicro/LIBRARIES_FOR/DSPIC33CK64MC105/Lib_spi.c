/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_spi.h"

#define LIB_SPI_TIMEOUT 60000U

/* rate = FP / (2 * (BRG + 1)) */
uint16_t LibSPI_GetPrescaler(SPI_TypeDef *SPIx, uint32_t MAX_SPI_Frequency)
{
    uint32_t div;

    (void)SPIx;
    if(MAX_SPI_Frequency == 0)
        return 0x1FFFU;
    div = (SystemCoreClock + 2UL * MAX_SPI_Frequency - 1UL) / (2UL * MAX_SPI_Frequency); /* rounded up */
    if(div < 1UL)
        div = 1UL;
    if(div > 0x2000UL)
        div = 0x2000UL;
    return (uint16_t)(div - 1UL);
}

void LibSPI_Init(SPI_TypeDef *SPIx, LibSPI_DataSize_t SPI_DataSize, uint16_t SPI_BaudRatePrescaler)
{
    (void)SPI_DataSize;

    if(SPIx != SPI1)
        return;

    SPI1CON1L = 0; /* module off */
    SPI1CON1H = 0;
    SPI1CON2L = 0;
    SPI1STATL = 0;
    SPI1BRGL = SPI_BaudRatePrescaler;

    /* RC0 (RP48) = SDO1, RC1 (RP49) = SDI1, RC2 (RP50) = SCK1: digital pins */
    ANSELC &= (uint16_t)~0x0007U;
    TRISC = (uint16_t)((TRISC & (uint16_t)~0x0005U) | 0x0002U);
    __builtin_write_RPCON(0x0000);
    RPINR20 = (uint16_t)((50U << 8) | 49U); /* SCK1 input = RP50, SDI1 = RP49 */
    __builtin_write_RPCON(0x0800);
    LibPPS_Output(48, LIB_PPS_SDO1);
    LibPPS_Output(50, LIB_PPS_SCK1);

    /* master, 8 bits, mode 0: clock idle low, data changes on the falling edge */
    SPI1CON1Lbits.MSTEN = 1;
    SPI1CON1Lbits.CKP = 0;
    SPI1CON1Lbits.CKE = 1;
    SPI1CON1Lbits.SPIEN = 1;
}

uint16_t LibSPI_Send(SPI_TypeDef *SPIx, uint16_t data)
{
    uint16_t n;

    (void)SPIx;
    if(SPI1CON1Lbits.SPIEN == 0)
        return 0;

    if(SPI1STATLbits.SPIROV)
        SPI1STATLbits.SPIROV = 0;
    SPI1BUFL = (uint8_t)data;
    for(n = 0; (SPI1STATLbits.SPIRBF == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    return (uint16_t)(SPI1BUFL & 0x00FFU);
}

void LibSPI_WriteMulti(SPI_TypeDef *SPIx, uint8_t *dataOut, uint32_t count)
{
    while(count--)
        (void)LibSPI_Send(SPIx, *dataOut++);
}
