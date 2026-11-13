/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_spi.h"

#define LIB_SPI_TIMEOUT 1000000U

uint16_t LibSPI_GetPrescaler(SPI_TypeDef *SPIx, uint32_t MAX_SPI_Frequency)
{
    uint32_t i;

    (void)SPIx; /* PCLK1 = PCLK2 = SystemCoreClock */

    if(MAX_SPI_Frequency == 0)
        return (uint16_t)(7U << SPI_CR1_BR_Pos);

    /* BR = 000 -> PCLK / 2, 001 -> PCLK / 4 ... 111 -> PCLK / 256 */
    for(i = 0; i < 8; i++) {
        if((SystemCoreClock >> (i + 1)) <= MAX_SPI_Frequency)
            return (uint16_t)(i << SPI_CR1_BR_Pos);
    }
    return (uint16_t)(7U << SPI_CR1_BR_Pos);
}

void LibSPI_Init(SPI_TypeDef *SPIx, LibSPI_DataSize_t SPI_DataSize, uint16_t SPI_BaudRatePrescaler)
{
    volatile uint32_t tmp;

    if(SPIx != SPI1)
        return;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    tmp = RCC->APB2ENR;
    (void)tmp;

    /* SCK = PA5, MISO = PA6, MOSI = PA7: AF5 */
    LibGPIO_InitAlternate(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_50MHz, 5);

    SPIx->CR1 = 0; /* disabled */
    if(SPI_DataSize == LibSPI_DataSize_16b)
        SPIx->CR2 = (15U << SPI_CR2_DS_Pos); /* 16 bits frames */
    else
        SPIx->CR2 = (7U << SPI_CR2_DS_Pos) | SPI_CR2_FRXTH; /* 8 bits frames, RXNE as soon as 1 byte is received */
    /* master, CPOL = 0, CPHA = 0, MSB first, software slave management */
    SPIx->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | (SPI_BaudRatePrescaler & SPI_CR1_BR);
    SPIx->CR1 |= SPI_CR1_SPE;
}

uint16_t LibSPI_Send(SPI_TypeDef *SPIx, uint16_t data)
{
    uint32_t n;
    int      wide = ((SPIx->CR2 & SPI_CR2_DS) == (15U << SPI_CR2_DS_Pos));

    if((SPIx->CR1 & SPI_CR1_SPE) == 0)
        return 0;

    for(n = 0; ((SPIx->SR & SPI_SR_TXE) == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    if(wide)
        SPIx->DR = data;
    else
        *(volatile uint8_t *)&SPIx->DR = (uint8_t)data; /* 8 bits access: only one byte goes to the FIFO */

    for(n = 0; ((SPIx->SR & SPI_SR_RXNE) == 0) && (n < LIB_SPI_TIMEOUT); n++)
        ;
    if((SPIx->SR & SPI_SR_RXNE) == 0)
        return 0;

    if(wide)
        return (uint16_t)SPIx->DR;
    return *(volatile uint8_t *)&SPIx->DR;
}

void LibSPI_WriteMulti(SPI_TypeDef *SPIx, uint8_t *dataOut, uint32_t count)
{
    while(count--)
        (void)LibSPI_Send(SPIx, *dataOut++);
}
