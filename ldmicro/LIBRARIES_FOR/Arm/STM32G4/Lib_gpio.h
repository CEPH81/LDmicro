/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_GPIO_H
#define LIB_GPIO_H

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Configures the pins of GPIO_Pin (bit mask) of one port.
 * GPIO_Mode : GPIO_Mode_IN or GPIO_Mode_OUT
 * GPIO_Type : GPIO_OType_PP / GPIO_OType_OD for outputs,
 *             GPIO_PuPd_NOPULL / GPIO_PuPd_UP / GPIO_PuPd_DOWN for inputs
 * GPIO_Speed: GPIO_Speed_2MHz ...
 */
void LibGPIO_Conf(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_Type, uint32_t GPIO_Speed);

/* Full configuration of the pins of GPIO_Pin (bit mask) */
void LibGPIO_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_OType, uint32_t GPIO_PuPd, uint32_t GPIO_Speed);

/* Pins of GPIO_Pin (bit mask) as alternate function number Alternate (0..15) */
void LibGPIO_InitAlternate(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_OType, uint32_t GPIO_PuPd, uint32_t GPIO_Speed, uint8_t Alternate);

/* Pins of GPIO_Pin (bit mask) as analog inputs */
void LibGPIO_InitAnalog(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/* Enables the clock of a port */
void LibGPIO_EnableClock(GPIO_TypeDef *GPIOx);

#ifdef __cplusplus
}
#endif

#endif
