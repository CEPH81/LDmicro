/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_adc.h"

#define LIB_ADC_TIMEOUT 1000000U

/* Port and pin of each analog channel; 0xFF = channel not available on the board */
static const uint8_t LibADC_Pins[26][2] = {
    {0, 0},       /* AN000 = P000 */
    {0, 1},       /* AN001 = P001 */
    {0, 2},       /* AN002 = P002 */
    {0, 3},       /* AN003 = P003 */
    {0, 4},       /* AN004 = P004 */
    {0xFF, 0xFF}, /* AN005 */
    {0xFF, 0xFF}, /* AN006 */
    {0xFF, 0xFF}, /* AN007 */
    {0xFF, 0xFF}, /* AN008 */
    {0, 14},      /* AN009 = P014 */
    {0xFF, 0xFF}, /* AN010 */
    {0xFF, 0xFF}, /* AN011 */
    {0xFF, 0xFF}, /* AN012 */
    {0xFF, 0xFF}, /* AN013 */
    {0xFF, 0xFF}, /* AN014 */
    {0xFF, 0xFF}, /* no channel 15 */
    {0xFF, 0xFF}, /* AN016 */
    {0xFF, 0xFF}, /* AN017 */
    {0xFF, 0xFF}, /* AN018 */
    {1, 3},       /* AN019 = P103 */
    {1, 2},       /* AN020 = P102 */
    {1, 1},       /* AN021 = P101 */
    {1, 0},       /* AN022 = P100 */
    {0xFF, 0xFF}, /* AN023 */
    {0xFF, 0xFF}, /* AN024 */
    {0xFF, 0xFF}, /* AN025 */
};

static int LibADC_Valid(uint8_t channel)
{
    return (channel < 26) && (LibADC_Pins[channel][0] != 0xFF);
}

void LibADC_Init(ADC_TypeDef *ADCx, uint8_t channel, uint32_t resol)
{
    (void)resol;

    if((ADCx != R_ADC0) || !LibADC_Valid(channel))
        return;

    /* Module stop of the ADC14: MSTPD16 */
    if(R_MSTP->MSTPCRD & (1UL << 16)) {
        R_SYSTEM->PRCR = LIB_PRCR_UNLOCK;
        R_MSTP->MSTPCRD &= ~(1UL << 16);
        R_SYSTEM->PRCR = LIB_PRCR_LOCK;

        ADCx->ADCSR = 0;   /* single scan mode, software trigger, no interrupt */
        ADCx->ADCER = 0;   /* 12 bits, right aligned */
        ADCx->ADADC = 0;   /* no addition or averaging */
    }

    LibGPIO_InitAnalog(LibADC_Pins[channel][0], LibADC_Pins[channel][1]);
}

uint16_t LibADC_Read(ADC_TypeDef *ADCx, uint8_t channel)
{
    uint32_t n;

    if((ADCx != R_ADC0) || !LibADC_Valid(channel) || (R_MSTP->MSTPCRD & (1UL << 16)))
        return 0;
    if(ADCx->ADCSR & 0x8000U)
        return 0; /* conversion in progress: should not happen */

    /* Select the channel: AN000..AN015 in ADANSA0, AN016..AN025 in ADANSA1 */
    if(channel < 16) {
        ADCx->ADANSA[0] = (uint16_t)(1U << channel);
        ADCx->ADANSA[1] = 0;
    } else {
        ADCx->ADANSA[0] = 0;
        ADCx->ADANSA[1] = (uint16_t)(1U << (channel - 16));
    }

    ADCx->ADCSR = 0x8000U; /* ADST = 1: start a single scan */
    for(n = 0; (ADCx->ADCSR & 0x8000U) && (n < LIB_ADC_TIMEOUT); n++)
        ;
    if(ADCx->ADCSR & 0x8000U)
        return 0;

    return (uint16_t)(ADCx->ADDR[channel] & 0x0FFFU);
}
