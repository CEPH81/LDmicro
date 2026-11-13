/**
 * LDmicro library for STM32L4 (STM32L432), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_ADC_H
#define LIB_ADC_H

/*
    ADC1 inputs of the STM32L432 (the channel number is the number of the ADC input):

    CHANNEL     PIN
    5           PA0
    6           PA1
    7           PA2
    8           PA3
    9           PA4
    10          PA5
    11          PA6
    12          PA7
    15          PB0
    16          PB1
*/

#include "Lib_mcu.h"
#include "Lib_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 12 bits, single conversion; resol is accepted for compatibility */
void     LibADC_Init(ADC_TypeDef *ADCx, uint8_t channel, uint32_t resol);
uint16_t LibADC_Read(ADC_TypeDef *ADCx, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
