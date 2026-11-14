/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_uart.h"

#define LIB_UART_TIMEOUT 60000U

void LibUart_Init(USART_TypeDef *USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops)
{
    uint32_t brg;

    (void)datalen;
    (void)parity;
    (void)stops;

    if(USARTx != USART1)
        return;
    if(baudrate == 0)
        baudrate = 9600;

    U1MODE = 0; /* UART off; asynchronous, 8 bits, no parity */
    U1MODEH = 0; /* clock = FP, 1 stop bit */
    U1STA = 0;

    /* Fractional baud rate generator: baud rate = FP / BRG, BRG of at least 16 */
    brg = (SystemCoreClock + baudrate / 2UL) / baudrate;
    if(brg < 16UL)
        brg = 16UL;
    U1MODEHbits.BCLKMOD = 1;
    U1BRG = (uint16_t)brg;
    U1BRGH = (uint16_t)((brg >> 16) & 0x000FU);

    /* RX on RC11 (RP59), TX on RC10 (RP58) */
    __builtin_write_RPCON(0x0000);
    RPINR18 = (uint16_t)((RPINR18 & 0xFF00U) | 59U); /* U1RX */
    __builtin_write_RPCON(0x0800);
    LibPPS_Output(58, LIB_PPS_U1TX);
    TRISC |= 0x0800U; /* RC11 input */

    U1MODEbits.UARTEN = 1;
    U1MODEbits.UTXEN = 1;
    U1MODEbits.URXEN = 1;
}

uint8_t LibUart_Transmit_Ready(USART_TypeDef *USARTx)
{
    (void)USARTx;
    return (uint8_t)(U1STAHbits.UTXBF ? 0 : 1); /* room in the transmit buffer */
}

uint8_t LibUart_Received_Data(USART_TypeDef *USARTx)
{
    (void)USARTx;
    if(U1STAbits.OERR)
        U1STAbits.OERR = 0; /* a receive overflow would block the receiver */
    return (uint8_t)(U1STAHbits.URXBE ? 0 : 1);
}

void LibUart_Putc(USART_TypeDef *USARTx, volatile char c)
{
    uint16_t n;

    for(n = 0; !LibUart_Transmit_Ready(USARTx) && (n < LIB_UART_TIMEOUT); n++)
        ;
    U1TXREG = (uint8_t)c;
}

uint8_t LibUart_Getc(USART_TypeDef *USARTx)
{
    uint16_t n;

    for(n = 0; !LibUart_Received_Data(USARTx) && (n < LIB_UART_TIMEOUT); n++)
        ;
    return (uint8_t)U1RXREG;
}
