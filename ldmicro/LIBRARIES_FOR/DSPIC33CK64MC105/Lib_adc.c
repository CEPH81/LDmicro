/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_adc.h"

#define LIB_ADC_TIMEOUT 60000U

typedef struct {
    uint8_t channel, port, pin; /* port: 0 = A, 1 = B, 2 = C */
} LibADC_Pin_t;

static const LibADC_Pin_t LibADC_Pins[] = {
    {0, 0, 0}, {1, 1, 2}, {2, 1, 7}, {3, 0, 3}, {4, 0, 4}, {5, 1, 0}, {6, 1, 1}, {8, 1, 3},
    {9, 0, 2}, {10, 1, 8}, {11, 1, 9}, {12, 2, 0}, {13, 2, 1}, {14, 2, 2}, {15, 2, 3},
};

static uint8_t LibADC_On = 0;

static uint16_t LibADC_Convert(uint8_t channel)
{
    uint16_t n, mask = (uint16_t)(1U << channel);

    (void)(&ADCBUF0)[channel]; /* reading the buffer clears the "ready" flag of the channel */
    ADCON3Lbits.CNVCHSEL = channel;
    ADCON3Lbits.CNVRTCH = 1; /* software trigger of this channel */
    for(n = 0; ((ADSTATL & mask) == 0) && (n < LIB_ADC_TIMEOUT); n++)
        ;
    return (uint16_t)((&ADCBUF0)[channel] & 0x0FFFU);
}

void LibADC_Init(ADC_TypeDef *ADCx, uint8_t channel, uint32_t resol)
{
    uint16_t n;
    uint8_t  i;

    (void)ADCx;
    (void)resol;

    for(i = 0; i < sizeof(LibADC_Pins) / sizeof(LibADC_Pins[0]); i++)
        if(LibADC_Pins[i].channel == channel)
            break;
    if(i >= sizeof(LibADC_Pins) / sizeof(LibADC_Pins[0]))
        return;

    /* analog input */
    *LibGPIO_Ports[LibADC_Pins[i].port].tris |= (uint16_t)(1U << LibADC_Pins[i].pin);
    *LibGPIO_Ports[LibADC_Pins[i].port].ansel |= (uint16_t)(1U << LibADC_Pins[i].pin);

    if(!LibADC_On) {
        ADCON1L = 0;      /* converter off during the configuration */
        ADCON1H = 0x0060; /* integer result, shared core: 12 bits */
        ADCON2L = 0;      /* shared core clock: source / 2 */
        ADCON2H = 10;     /* shared core sampling time: 12 clocks */
        ADCON3L = 0;      /* reference = AVDD and AVSS */
        ADCON3H = 0;      /* clock source: peripheral clock, not divided */
        ADCON5H = 0x0F00; /* longest warm-up time */

        ADCON1Lbits.ADON = 1;
        ADCON5Lbits.SHRPWR = 1; /* power of the shared core */
        for(n = 0; (ADCON5Lbits.SHRRDY == 0) && (n < LIB_ADC_TIMEOUT); n++)
            ;
        ADCON3Hbits.SHREN = 1; /* shared core enabled */
        LibADC_On = 1;
    }
}

uint16_t LibADC_Read(ADC_TypeDef *ADCx, uint8_t channel)
{
    (void)ADCx;
    if(!LibADC_On || (channel > 15U))
        return 0;
    return LibADC_Convert(channel);
}
