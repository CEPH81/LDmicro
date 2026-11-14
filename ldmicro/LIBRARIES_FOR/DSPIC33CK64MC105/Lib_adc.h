/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_ADC_H
#define LIB_ADC_H

/*
    Inputs of the A/D converter (the channel is the ANx number):

    CHANNEL     PIN         CHANNEL     PIN         CHANNEL     PIN
    0           RA0         5           RB0         11          RB9
    1           RB2         6           RB1         12          RC0
    2           RB7         8           RB3         13          RC1
    3           RA3         9           RA2         14          RC2
    4           RA4         10          RB8         15          RC3

    Reference: the analog supply of the MCU (AVDD). Result on 12 bits (0..4095).
*/

#include "Lib_mcu.h"

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
