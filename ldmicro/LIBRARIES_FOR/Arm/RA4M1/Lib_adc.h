/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_ADC_H
#define LIB_ADC_H

/*
    Inputs of the A/D converter on the Arduino Nano R4 (the channel is the ANxxx number):

    CHANNEL     PIN             CHANNEL     PIN
    9           P014 (A0)       21          P101 (A4)
    0           P000 (A1)       22          P100 (A5)
    1           P001 (A2)       4           P004 (A6)
    2           P002 (A3)       3           P003 (A7)
    19          P103 (D4)       20          P102 (D5)
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
