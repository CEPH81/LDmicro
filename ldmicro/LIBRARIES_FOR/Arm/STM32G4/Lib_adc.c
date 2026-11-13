/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_adc.h"
#include "Lib_usr.h"

#define LIB_ADC_TIMEOUT 1000000U

/* Bits of ADC_CR that are set by writing 1: they must be written as 0 when another bit is modified */
#define ADC_CR_RS_BITS (ADC_CR_ADCAL | ADC_CR_JADSTP | ADC_CR_ADSTP | ADC_CR_JADSTART | ADC_CR_ADSTART | ADC_CR_ADDIS | ADC_CR_ADEN)

static void LibADC_InitPin(ADC_TypeDef *ADCx, uint8_t channel)
{
    if(ADCx == ADC1) {
        switch(channel) {
            case 1:
            case 2:
            case 3:
            case 4:
                LibGPIO_InitAnalog(GPIOA, (uint16_t)(1U << (channel - 1))); /* PA0..PA3 */
                break;
            case 5:
                LibGPIO_InitAnalog(GPIOB, GPIO_Pin_14);
                break;
            case 11:
                LibGPIO_InitAnalog(GPIOB, GPIO_Pin_12);
                break;
            case 12:
                LibGPIO_InitAnalog(GPIOB, GPIO_Pin_1);
                break;
            case 14:
                LibGPIO_InitAnalog(GPIOB, GPIO_Pin_11);
                break;
            case 15:
                LibGPIO_InitAnalog(GPIOB, GPIO_Pin_0);
                break;
            default:
                break;
        }
    } else {
        switch(channel) {
            case 3:
                LibGPIO_InitAnalog(GPIOA, GPIO_Pin_6);
                break;
            case 4:
                LibGPIO_InitAnalog(GPIOA, GPIO_Pin_7);
                break;
            case 5:
                LibGPIO_InitAnalog(GPIOC, GPIO_Pin_4);
                break;
            case 12:
                LibGPIO_InitAnalog(GPIOB, GPIO_Pin_2);
                break;
            case 13:
                LibGPIO_InitAnalog(GPIOA, GPIO_Pin_5);
                break;
            case 15:
                LibGPIO_InitAnalog(GPIOB, GPIO_Pin_15);
                break;
            default:
                break;
        }
    }
}

static void LibADC_InitADC(ADC_TypeDef *ADCx)
{
    volatile uint32_t tmp;
    uint32_t          n;

    if(ADCx->CR & ADC_CR_ADEN)
        return; /* already done */

    RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;
    tmp = RCC->AHB2ENR;
    (void)tmp;

    /* ADC clock = HCLK / 4 (synchronous mode): 20 MHz when HCLK = 80 MHz.
       ADC1 and ADC2 share this setting: it is written while both are disabled. */
    if(((ADC1->CR | ADC2->CR) & ADC_CR_ADEN) == 0)
        ADC12_COMMON->CCR = (ADC12_COMMON->CCR & ~ADC_CCR_CKMODE) | (3U << ADC_CCR_CKMODE_Pos);

    /* leave deep power down, then start the voltage regulator */
    ADCx->CR &= ~(ADC_CR_DEEPPWD | ADC_CR_RS_BITS);
    ADCx->CR = (ADCx->CR & ~ADC_CR_RS_BITS) | ADC_CR_ADVREGEN;
    delay_us(50); /* regulator start-up time: 20 us max */

    /* 12 bits, right aligned, single conversion, software start */
    ADCx->CFGR = 0;
    ADCx->CFGR2 = 0;
    /* sampling time 92.5 ADC clock periods on every channel */
    ADCx->SMPR1 = 0x2DB6DB6DU; /* SMP0..SMP9   = 101b */
    ADCx->SMPR2 = 0x05B6DB6DU; /* SMP10..SMP18 = 101b */

    /* calibration in single-ended mode */
    ADCx->CR = (ADCx->CR & ~(ADC_CR_RS_BITS | ADC_CR_ADCALDIF)) | ADC_CR_ADCAL;
    for(n = 0; (ADCx->CR & ADC_CR_ADCAL) && (n < LIB_ADC_TIMEOUT); n++)
        ;
    delay_us(10); /* at least 4 ADC clock periods before enabling the ADC */

    /* enable */
    ADCx->ISR = ADC_ISR_ADRDY;
    ADCx->CR = (ADCx->CR & ~ADC_CR_RS_BITS) | ADC_CR_ADEN;
    for(n = 0; ((ADCx->ISR & ADC_ISR_ADRDY) == 0) && (n < LIB_ADC_TIMEOUT); n++)
        ;
}

void LibADC_Init(ADC_TypeDef *ADCx, uint8_t channel, uint32_t resol)
{
    (void)resol;

    if((ADCx != ADC1) && (ADCx != ADC2))
        return;
    LibADC_InitPin(ADCx, channel);
    LibADC_InitADC(ADCx);
}

uint16_t LibADC_Read(ADC_TypeDef *ADCx, uint8_t channel)
{
    uint32_t n;

    if(((ADCx != ADC1) && (ADCx != ADC2)) || (channel > 18) || ((ADCx->CR & ADC_CR_ADEN) == 0))
        return 0;
    if(ADCx->CR & ADC_CR_ADSTART)
        return 0; /* conversion in progress: should not happen */

    ADCx->SQR1 = ((uint32_t)channel << ADC_SQR1_SQ1_Pos); /* sequence of 1 conversion */
    ADCx->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
    ADCx->CR = (ADCx->CR & ~ADC_CR_RS_BITS) | ADC_CR_ADSTART;
    for(n = 0; ((ADCx->ISR & ADC_ISR_EOC) == 0) && (n < LIB_ADC_TIMEOUT); n++)
        ;
    if((ADCx->ISR & ADC_ISR_EOC) == 0)
        return 0;

    return (uint16_t)(ADCx->DR & 0x0FFFU);
}
