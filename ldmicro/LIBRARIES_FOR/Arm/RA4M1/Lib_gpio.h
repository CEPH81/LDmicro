/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_GPIO_H
#define LIB_GPIO_H

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Configures the pins of GPIO_Pin (bit mask) of one port as plain inputs or outputs.
 * GPIO_Mode : GPIO_Mode_IN or GPIO_Mode_OUT
 * GPIO_Type : GPIO_OType_PP / GPIO_OType_OD for outputs,
 *             GPIO_PuPd_NOPULL / GPIO_PuPd_UP for inputs (there are no pull-down resistors)
 * GPIO_Speed: not used
 */
void LibGPIO_Conf(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_Type, uint32_t GPIO_Speed);

/* Writes value to the pins configured as outputs with LibGPIO_Conf(); the other pins are not changed */
void LibGPIO_Write(GPIO_TypeDef *GPIOx, uint16_t value);

/* Gives one pin to a peripheral: psel = LIB_PSEL_xxx */
void LibGPIO_InitPeripheral(uint32_t port, uint32_t pin, uint32_t psel);

/* One pin as analog input */
void LibGPIO_InitAnalog(uint32_t port, uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif
