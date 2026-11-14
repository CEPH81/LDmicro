/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_GPIO_H
#define LIB_GPIO_H

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Same arguments as the libraries of the ARM targets:
 *   LibGPIO_Conf(GPIOx, pins, GPIO_Mode_OUT, GPIO_OType_PP, speed)
 *   LibGPIO_Conf(GPIOx, pins, GPIO_Mode_IN, GPIO_PuPd_UP or GPIO_PuPd_NOPULL, speed)
 */
void LibGPIO_Conf(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t GPIO_Mode, uint8_t GPIO_Type, uint8_t GPIO_Speed);

/* Writes the pins that LibGPIO_Conf() configured as outputs */
void LibGPIO_Write(GPIO_TypeDef *GPIOx, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
