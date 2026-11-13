/**
 * LDmicro library for STM32L4 (STM32L432), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_USR_H
#define LIB_USR_H

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t swap(uint16_t var);
int16_t  opposite(int16_t var);
uint8_t  bcd2bin(uint8_t var);
uint8_t  bin2bcd(uint8_t var);

/* Delays measured with the cycle counter of the core (enabled by SystemInit()) */
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);

#ifdef __cplusplus
}
#endif

#endif
