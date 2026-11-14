/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_ADC_H
#define LIB_ADC_H

/*
    Inputs of the A/D converter (the channel is the AINx number):

    CHANNEL     PIN         CHANNEL     PIN         CHANNEL     PIN
    0           PA02        5           PA07        10          PB02
    1           PA03        6           PA08        11          PB03
    2           PA04        7           PA09        18          PB08
    3           PA05        8           PA10        19          PB09
    4           PA06        9           PA11

    Reference: the analog supply of the MCU (INTVCC2). Result on 12 bits.
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
