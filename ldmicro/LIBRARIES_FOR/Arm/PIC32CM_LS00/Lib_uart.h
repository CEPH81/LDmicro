/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_UART_H
#define LIB_UART_H

/*
    Pins used:

    USARTx      TX              RX
    USART3      PB08            PB09            SERCOM3

    On the Curiosity Nano board these two pins go to the virtual COM port of the debugger:
    the PLC program talks to the PC through the USB cable of the board.
*/

#include "Lib_mcu.h"
#include "Lib_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 8 data bits, no parity, 1 stop bit: datalen, parity and stops are accepted for compatibility */
void    LibUart_Init(USART_TypeDef *USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops);
uint8_t LibUart_Getc(USART_TypeDef *USARTx);
void    LibUart_Putc(USART_TypeDef *USARTx, volatile char c);
uint8_t LibUart_Transmit_Ready(USART_TypeDef *USARTx);
uint8_t LibUart_Received_Data(USART_TypeDef *USARTx);

#ifdef __cplusplus
}
#endif

#endif
