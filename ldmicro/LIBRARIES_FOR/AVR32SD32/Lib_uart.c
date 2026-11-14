/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_uart.h"

#define LIB_UART_TIMEOUT 60000U

void LibUart_Init(USART_TypeDef *USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops)
{
    uint32_t baud;

    (void)datalen;
    (void)parity;
    (void)stops;

    if(USARTx != USART0)
        return;
    if(baudrate == 0)
        baudrate = 9600;

    /* BAUD = 64 * clock / (16 * baud rate), with its 6 lower bits as the fractional part */
    baud = (4UL * SystemCoreClock + baudrate / 2UL) / baudrate;
    if(baud < 64UL)
        baud = 64UL;
    if(baud > 65535UL)
        baud = 65535UL;

    USARTx->CTRLB = 0;

    /* USART0 on PD4 (TX), PD5 (RX) */
    PORTMUX.USARTROUTEA = (uint8_t)((PORTMUX.USARTROUTEA & (uint8_t)~PORTMUX_USART0_gm) | PORTMUX_USART0_ALT3_gc);
    PORTD.OUTSET = PIN4_bm; /* TX idle level */
    PORTD.DIRSET = PIN4_bm;
    PORTD.DIRCLR = PIN5_bm;

    USARTx->BAUD = (uint16_t)baud;
    USARTx->CTRLC = USART_CHSIZE_8BIT_gc; /* asynchronous, no parity, 1 stop bit, 8 bits */
    USARTx->CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}

uint8_t LibUart_Transmit_Ready(USART_TypeDef *USARTx)
{
    return (uint8_t)((USARTx->STATUS & USART_DREIF_bm) ? 1 : 0);
}

uint8_t LibUart_Received_Data(USART_TypeDef *USARTx)
{
    return (uint8_t)((USARTx->STATUS & USART_RXCIF_bm) ? 1 : 0);
}

void LibUart_Putc(USART_TypeDef *USARTx, volatile char c)
{
    uint16_t n;

    for(n = 0; !LibUart_Transmit_Ready(USARTx) && (n < LIB_UART_TIMEOUT); n++)
        ;
    USARTx->TXDATAL = (uint8_t)c;
}

uint8_t LibUart_Getc(USART_TypeDef *USARTx)
{
    uint16_t n;

    for(n = 0; !LibUart_Received_Data(USARTx) && (n < LIB_UART_TIMEOUT); n++)
        ;
    return USARTx->RXDATAL; /* reading the data also clears the error flags of the character */
}
