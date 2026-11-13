/**
 * LDmicro library for STM32L4 (STM32L432), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_pwm.h"

static int LibPWM_Channels(TIM_TypeDef *TIMx)
{
    if((TIMx == TIM1) || (TIMx == TIM2))
        return 4;
    if(TIMx == TIM15)
        return 2;
    return 0;
}

static void LibPWM_InitPin(TIM_TypeDef *TIMx, LibPWM_Channel_t Channel)
{
    if(TIMx == TIM1)
        LibGPIO_InitAlternate(GPIOA, (uint16_t)(GPIO_Pin_8 << Channel), GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_50MHz, 1); /* PA8..PA11: AF1 */
    else if(TIMx == TIM2)
        LibGPIO_InitAlternate(GPIOA, (uint16_t)(GPIO_Pin_0 << Channel), GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_50MHz, 1); /* PA0..PA3: AF1 */
    else if(TIMx == TIM15)
        LibGPIO_InitAlternate(GPIOA, (uint16_t)(GPIO_Pin_2 << Channel), GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_50MHz, 14); /* PA2, PA3: AF14 */
}

LibPWM_Result_t LibPWM_InitTimer(TIM_TypeDef *TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency)
{
    uint32_t freq, counts, psc, maxCount;

    TIM_Data->TIM = TIMx;

    if(LibPWM_Channels(TIMx) == 0)
        return LibPWM_Result_TimerNotValid;
    LibTimer_EnableClock(TIMx);

    if(PWMFrequency < 1.0) {
        /* PWM stopped: the outputs of this structure are driven low by LibPWM_SetChannelPercent() */
        TIM_Data->Period = 0;
        TIM_Data->Frequency = 0;
        TIM_Data->Micros = 0;
        return LibPWM_Result_FrequencyTooLow;
    }
    if(PWMFrequency > (double)(SystemCoreClock / 100U))
        return LibPWM_Result_FrequencyTooHigh; /* less than 100 steps of duty cycle */

    freq = (uint32_t)PWMFrequency;
    counts = SystemCoreClock / freq; /* timer clock periods in one PWM period */

    maxCount = (TIMx == TIM2) ? 0x80000000U : 0x10000U; /* TIM2 is a 32 bits counter */
    psc = 1;
    while((counts / psc) > maxCount) {
        psc++;
        if(psc > 0x10000U)
            return LibPWM_Result_FrequencyTooLow;
    }

    TIM_Data->Prescaler = psc - 1;
    TIM_Data->Period = counts / psc;
    TIM_Data->Frequency = freq;
    TIM_Data->Micros = 1000000U / freq;

    TIMx->PSC = TIM_Data->Prescaler;
    TIMx->ARR = TIM_Data->Period - 1;
    TIMx->CR1 |= TIM_CR1_ARPE;
    if((TIMx == TIM1) || (TIMx == TIM15))
        TIMx->BDTR |= TIM_BDTR_MOE; /* main output enable */
    TIMx->EGR = TIM_EGR_UG;         /* load prescaler and period now */
    TIMx->CR1 |= TIM_CR1_CEN;

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_InitChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, LibPWM_PinsPack_t PinsPack)
{
    TIM_TypeDef *TIMx = TIM_Data->TIM;
    uint32_t     shift;

    (void)PinsPack; /* only one pins pack */

    if(TIMx == 0)
        return LibPWM_Result_TimerNotValid;
    if((int)Channel >= LibPWM_Channels(TIMx))
        return LibPWM_Result_ChannelNotValid;

    /* PWM mode 1 (OCxM = 110) with preload of the compare register */
    shift = ((uint32_t)Channel & 1U) * 8U;
    if(Channel < LibPWM_Channel_3) {
        TIMx->CCMR1 = (TIMx->CCMR1 & ~(0xFFU << shift)) | ((0x60U | TIM_CCMR1_OC1PE) << shift);
        if(Channel == LibPWM_Channel_1)
            TIMx->CCR1 = 0;
        else
            TIMx->CCR2 = 0;
    } else {
        TIMx->CCMR2 = (TIMx->CCMR2 & ~(0xFFU << shift)) | ((0x60U | TIM_CCMR1_OC1PE) << shift);
        if(Channel == LibPWM_Channel_3)
            TIMx->CCR3 = 0;
        else
            TIMx->CCR4 = 0;
    }
    /* output enabled, active high */
    TIMx->CCER = (TIMx->CCER & ~(0xFU << (4U * (uint32_t)Channel))) | (TIM_CCER_CC1E << (4U * (uint32_t)Channel));

    LibPWM_InitPin(TIMx, Channel);
    TIM_Data->CH_Init |= (uint8_t)(1U << Channel);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_SetChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, uint32_t Pulse)
{
    TIM_TypeDef *TIMx = TIM_Data->TIM;

    if(TIMx == 0)
        return LibPWM_Result_TimerNotValid;
    if((int)Channel >= LibPWM_Channels(TIMx))
        return LibPWM_Result_ChannelNotValid;
    if(Pulse > TIM_Data->Period)
        Pulse = TIM_Data->Period; /* 100 % */

    switch(Channel) {
        case LibPWM_Channel_1:
            TIMx->CCR1 = Pulse;
            break;
        case LibPWM_Channel_2:
            TIMx->CCR2 = Pulse;
            break;
        case LibPWM_Channel_3:
            TIMx->CCR3 = Pulse;
            break;
        default:
            TIMx->CCR4 = Pulse;
            break;
    }
    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_SetChannelPercent(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, int32_t percent)
{
    uint32_t pulse;

    if(percent < 0)
        percent = 0;
    if(percent > 100)
        percent = 100;

    if(TIM_Data->Period > 40000000U)
        pulse = (TIM_Data->Period / 100U) * (uint32_t)percent; /* no overflow with the 32 bits timer */
    else
        pulse = (TIM_Data->Period * (uint32_t)percent) / 100U;

    return LibPWM_SetChannel(TIM_Data, Channel, pulse);
}
