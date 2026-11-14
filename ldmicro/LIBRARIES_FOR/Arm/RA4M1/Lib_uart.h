/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_UART_H
#define LIB_UART_H

/*
    Pins used:

    USARTx      TX              RX
    USART2      P302 (D1)       P301 (D0)       SCI2
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
