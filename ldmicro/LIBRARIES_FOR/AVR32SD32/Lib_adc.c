/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_adc.h"

#define LIB_ADC_TIMEOUT 60000U

static uint16_t LibADC_Convert(ADC_TypeDef *ADCx, uint8_t channel)
{
    uint16_t n;

    ADCx->MUXPOS = channel;
    ADCx->INTFLAGS = ADC_RESRDY_bm; /* clear the flag */
    ADCx->COMMAND = ADC_MODE_SINGLE_10BIT_gc | ADC_START_IMMEDIATE_gc;
    for(n = 0; ((ADCx->INTFLAGS & ADC_RESRDY_bm) == 0) && (n < LIB_ADC_TIMEOUT); n++)
        ;
    return (uint16_t)(ADCx->RESULT & 0x03FFU);
}

void LibADC_Init(ADC_TypeDef *ADCx, uint8_t channel, uint32_t resol)
{
    PORT_t *port;
    uint8_t pin;

    (void)resol;

    if((channel >= 1U) && (channel <= 7U)) {
        port = &PORTD;
        pin = channel;
    } else if((channel >= 16U) && (channel <= 21U)) {
        port = &PORTF;
        pin = (uint8_t)(channel - 16U);
    } else if((channel >= 22U) && (channel <= 27U)) {
        port = &PORTA;
        pin = (uint8_t)(channel - 20U);
    } else if((channel >= 28U) && (channel <= 31U)) {
        port = &PORTC;
        pin = (uint8_t)(channel - 28U);
    } else {
        return;
    }

    /* analog input: pin as input, digital input buffer and pull-up off */
    port->DIRCLR = (uint8_t)(1U << pin);
    (&port->PIN0CTRL)[pin] = PORT_ISC_INPUT_DISABLE_gc;

    if(ADCx->CTRLA & ADC_ENABLE_bm)
        return; /* converter already enabled by a previous call */

    ADCx->CTRLB = ADC_PRESC_DIV16_gc; /* converter clock = 16 MHz / 16 = 1 MHz */
    ADCx->CTRLC = ADC_REFSEL_VDD_gc;  /* reference = VDD */
    ADCx->CTRLE = 15;                 /* sample duration */
    ADCx->CTRLA = ADC_ENABLE_bm;

    /* the first conversion after the start of the converter is not used */
    (void)LibADC_Convert(ADCx, channel);
}

uint16_t LibADC_Read(ADC_TypeDef *ADCx, uint8_t channel)
{
    if((ADCx->CTRLA & ADC_ENABLE_bm) == 0)
        return 0;
    return LibADC_Convert(ADCx, channel);
}
