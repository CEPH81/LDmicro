/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_uart.h"

#define LIB_UART_TIMEOUT 100000U

#define USART_DRE 0x01U
#define USART_RXC 0x04U
#define USART_ERROR 0x80U

static void LibUart_Sync(USART_TypeDef *USARTx)
{
    uint32_t n;

    for(n = 0; USARTx->SYNCBUSY && (n < LIB_UART_TIMEOUT); n++)
        ;
}

void LibUart_Init(USART_TypeDef *USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops)
{
    uint32_t div8;

    (void)datalen;
    (void)parity;
    (void)stops;

    if(USARTx != USART3)
        return;
    if(baudrate == 0)
        baudrate = 9600;

    LibClock_Enable(LIB_GCLK_SERCOM3);

    USARTx->CTRLA = LIB_SERCOM_SWRST;
    LibUart_Sync(USARTx);

    /*
     * Fractional baud rate generator, 16 samples per bit:
     *   clock / (16 * baud) = BAUD + FP / 8
     */
    div8 = ((SystemCoreClock / 2U) + (baudrate / 2U)) / baudrate; /* = 8 * clock / (16 * baud) */
    if(div8 < 8U)
        div8 = 8U;
    if(div8 > 0xFFFFU)
        div8 = 0xFFFFU;

    /* internal clock, fractional generator, TX on PAD0, RX on PAD1, LSB first */
    USARTx->CTRLA = (1UL << 2) | (1UL << 13) | (0UL << 16) | (1UL << 20) | (1UL << 30);
    USARTx->BAUD16 = (uint16_t)((div8 >> 3) | ((div8 & 7U) << 13));
    USARTx->CTRLB = (1UL << 16) | (1UL << 17); /* TXEN, RXEN; 8 bits, 1 stop bit, no parity */
    LibUart_Sync(USARTx);

    /* PB08 = SERCOM3 PAD0 (TX), PB09 = SERCOM3 PAD1 (RX): function D */
    LibGPIO_InitPeripheral(1, 8, LIB_PMUX_D);
    LibGPIO_InitPeripheral(1, 9, LIB_PMUX_D);

    USARTx->CTRLA |= LIB_SERCOM_ENABLE;
    LibUart_Sync(USARTx);
}

uint8_t LibUart_Transmit_Ready(USART_TypeDef *USARTx)
{
    return (uint8_t)((USARTx->INTFLAG & USART_DRE) ? 1 : 0);
}

uint8_t LibUart_Received_Data(USART_TypeDef *USARTx)
{
    /* parity, framing and overflow errors would block the receiver: clear them */
    if(USARTx->INTFLAG & USART_ERROR) {
        USARTx->STATUS = 0x07U;
        USARTx->INTFLAG = USART_ERROR;
    }
    return (uint8_t)((USARTx->INTFLAG & USART_RXC) ? 1 : 0);
}

void LibUart_Putc(USART_TypeDef *USARTx, volatile char c)
{
    uint32_t n;

    for(n = 0; !LibUart_Transmit_Ready(USARTx) && (n < LIB_UART_TIMEOUT); n++)
        ;
    USARTx->DATA = (uint8_t)c;
}

uint8_t LibUart_Getc(USART_TypeDef *USARTx)
{
    uint32_t n;

    for(n = 0; !LibUart_Received_Data(USARTx) && (n < LIB_UART_TIMEOUT); n++)
        ;
    return (uint8_t)USARTx->DATA;
}
