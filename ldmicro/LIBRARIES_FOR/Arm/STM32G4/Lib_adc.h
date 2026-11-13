/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_ADC_H
#define LIB_ADC_H

/*
    ADC inputs of the STM32G431 (the channel number is the number of the ADC input):

    ADC1 CHANNEL    PIN         ADC2 CHANNEL    PIN
    1               PA0         3               PA6
    2               PA1         4               PA7
    3               PA2         5               PC4
    4               PA3         12              PB2
    5               PB14        13              PA5
    11              PB12        15              PB15
    12              PB1
    14              PB11
    15              PB0
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
