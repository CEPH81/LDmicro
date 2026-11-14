/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_ADC_H
#define LIB_ADC_H

/*
    Inputs of the A/D converter ADC0 (the channel is the AINx number):

    CHANNEL     PINS
    1..7        PD1..PD7
    16..21      PF0..PF5
    22..27      PA2..PA7
    28..31      PC0..PC3

    Reference: the supply of the MCU (VDD). Result on 10 bits (0..1023).
*/

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 10 bits, single conversion; resol is accepted for compatibility */
void     LibADC_Init(ADC_TypeDef *ADCx, uint8_t channel, uint32_t resol);
uint16_t LibADC_Read(ADC_TypeDef *ADCx, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
