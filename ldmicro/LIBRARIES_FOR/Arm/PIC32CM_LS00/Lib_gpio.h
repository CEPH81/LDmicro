/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_GPIO_H
#define LIB_GPIO_H

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Same arguments as the STM32F4 library:
 *   LibGPIO_Conf(GPIOx, pins, GPIO_Mode_OUT, GPIO_OType_PP, speed)
 *   LibGPIO_Conf(GPIOx, pins, GPIO_Mode_IN, GPIO_PuPd_UP or GPIO_PuPd_NOPULL, speed)
 */
void LibGPIO_Conf(const GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_Type, uint32_t GPIO_Speed);

/* Writes the pins that LibGPIO_Conf() configured as outputs */
void LibGPIO_Write(const GPIO_TypeDef *GPIOx, uint16_t value);

/* Gives a pin to a peripheral: group 0 = PA, 1 = PB; pin 0..31; pmux = LIB_PMUX_x */
void LibGPIO_InitPeripheral(uint32_t group, uint32_t pin, uint32_t pmux);

#ifdef __cplusplus
}
#endif

#endif
