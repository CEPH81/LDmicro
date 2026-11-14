/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_adc.h"

#define LIB_ADC_TIMEOUT 100000U

typedef struct {
    uint8_t channel, group, pin;
} LibADC_Pin_t;

static const LibADC_Pin_t LibADC_Pins[] = {
    {0, 0, 2}, {1, 0, 3}, {2, 0, 4}, {3, 0, 5}, {4, 0, 6}, {5, 0, 7}, {6, 0, 8},
    {7, 0, 9}, {8, 0, 10}, {9, 0, 11}, {10, 1, 2}, {11, 1, 3}, {18, 1, 8}, {19, 1, 9},
};

static void LibADC_Sync(ADC_TypeDef *ADCx)
{
    uint32_t n;

    for(n = 0; ADCx->SYNCBUSY && (n < LIB_ADC_TIMEOUT); n++)
        ;
}

static uint16_t LibADC_Convert(ADC_TypeDef *ADCx, uint8_t channel)
{
    uint32_t n;

    /* positive input = AINx, negative input = internal ground */
    ADCx->INPUTCTRL = (uint16_t)(channel | (0x18U << 8));
    LibADC_Sync(ADCx);
    ADCx->INTFLAG = 0x01U; /* clear RESRDY */
    ADCx->SWTRIG = 0x02U;  /* start */
    for(n = 0; ((ADCx->INTFLAG & 0x01U) == 0) && (n < LIB_ADC_TIMEOUT); n++)
        ;
    return (uint16_t)(ADCx->RESULT & 0x0FFFU);
}

void LibADC_Init(ADC_TypeDef *ADCx, uint8_t channel, uint32_t resol)
{
    uint32_t i;

    (void)resol;

    for(i = 0; i < sizeof(LibADC_Pins) / sizeof(LibADC_Pins[0]); i++)
        if(LibADC_Pins[i].channel == channel)
            break;
    if(i >= sizeof(LibADC_Pins) / sizeof(LibADC_Pins[0]))
        return;

    /* analog function of the pin (B) */
    LibGPIO_InitPeripheral(LibADC_Pins[i].group, LibADC_Pins[i].pin, LIB_PMUX_B);

    if(ADCx->CTRLA & 0x02U)
        return; /* converter already enabled by a previous call */

    LibClock_Enable(LIB_GCLK_ADC);

    ADCx->CTRLA = 0x01U; /* software reset */
    LibADC_Sync(ADCx);

    /* factory calibration */
    ADCx->CALIB = (uint16_t)(((LIB_SW_CALIB >> 3) & 0x07U) | ((LIB_SW_CALIB & 0x07U) << 8));
    ADCx->CTRLB = 0x03U;   /* clock = 16 MHz / 16 = 1 MHz */
    ADCx->REFCTRL = 0x05U; /* reference = analog supply (INTVCC2) */
    ADCx->CTRLC = 0;       /* 12 bits, single ended, single conversion */
    ADCx->SAMPCTRL = 15U;  /* sampling time: 16 clocks */
    LibADC_Sync(ADCx);
    ADCx->CTRLA = 0x02U; /* enable */
    LibADC_Sync(ADCx);

    /* the first conversion after the start of the converter is not reliable */
    (void)LibADC_Convert(ADCx, channel);
}

uint16_t LibADC_Read(ADC_TypeDef *ADCx, uint8_t channel)
{
    if((ADCx->CTRLA & 0x02U) == 0)
        return 0;
    return LibADC_Convert(ADCx, channel);
}
