/**
 * LDmicro library for STM32L4 (STM32L432), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_UART_H
#define LIB_UART_H

/*
    Pins used:

    USARTx      TX      RX
    USART1      PA9     PA10
    USART2      PA2     PA15    (virtual COM port of the ST-Link on the Nucleo-L432KC)
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
