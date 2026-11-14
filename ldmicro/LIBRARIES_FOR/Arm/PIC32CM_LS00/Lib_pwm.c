/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_pwm.h"

#define LIB_PWM_TIMEOUT 100000U

typedef struct {
    uint8_t group, pin, pmux; /* pmux = 0: no output */
} LibPWM_Pin_t;

/* Output pin of each compare channel. Outputs WO4..WO7 of a TCC repeat the channels 0..3:
   PA15 is the output WO5 of TCC0, that is the channel 1 */
static const LibPWM_Pin_t LibPWM_Pins[4][4] = {
    {{0, 8, LIB_PMUX_E}, {0, 15, LIB_PMUX_J}, {0, 18, LIB_PMUX_J}, {0, 19, LIB_PMUX_J}},
    {{0, 10, LIB_PMUX_E}, {0, 11, LIB_PMUX_E}, {0, 0, 0}, {0, 0, 0}},
    {{0, 12, LIB_PMUX_E}, {0, 13, LIB_PMUX_E}, {0, 0, 0}, {0, 0, 0}},
    {{1, 22, LIB_PMUX_J}, {1, 23, LIB_PMUX_J}, {1, 2, LIB_PMUX_J}, {1, 3, LIB_PMUX_J}},
};

static Lib_Tcc_t *const LibPWM_Tcc[4] = {LIB_TCC0, LIB_TCC1, LIB_TCC2, LIB_TCC3};
static const uint8_t    LibPWM_Clock[4] = {LIB_GCLK_TCC0_TCC1, LIB_GCLK_TCC0_TCC1, LIB_GCLK_TCC2, LIB_GCLK_TCC3};

static void LibPWM_Sync(Lib_Tcc_t *tcc)
{
    uint32_t n;

    for(n = 0; tcc->SYNCBUSY && (n < LIB_PWM_TIMEOUT); n++)
        ;
}

LibPWM_Result_t LibPWM_InitTimer(uint32_t TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency)
{
    Lib_Tcc_t *tcc;
    uint32_t   frequency, clocks, counts, prescaler = 0;

    TIM_Data->Valid = 0;
    if(TIMx > TIM3)
        return LibPWM_Result_TimerNotValid;
    tcc = LibPWM_Tcc[TIMx];
    TIM_Data->TIM = TIMx;

    LibClock_Enable(LibPWM_Clock[TIMx]);

    /* stop the counter: the outputs go low */
    tcc->CTRLA &= ~0x02UL;
    LibPWM_Sync(tcc);

    TIM_Data->Valid = 1;
    TIM_Data->Period = 0;
    TIM_Data->Frequency = 0;
    TIM_Data->Micros = 0;

    if(!(PWMFrequency >= 1.0))
        return (PWMFrequency > 0.0) ? LibPWM_Result_FrequencyTooLow : LibPWM_Result_Ok; /* <= 0: PWM stopped */
    if(PWMFrequency > (double)(SystemCoreClock / 2U))
        return LibPWM_Result_FrequencyTooHigh;

    frequency = (uint32_t)PWMFrequency;
    clocks = SystemCoreClock / frequency;
    counts = LibTimer_Divide(clocks, (TIMx == TIM2) ? 0x10000UL : 0x1000000UL, &prescaler);
    if(counts == 0)
        return LibPWM_Result_FrequencyTooLow;

    TIM_Data->Period = counts;
    TIM_Data->Prescaler = prescaler;
    TIM_Data->Frequency = frequency;
    TIM_Data->Micros = 1000000UL / frequency;

    tcc->CTRLA = (prescaler << 8) | (1UL << 12); /* prescaler, counter reloaded on the prescaler clock */
    tcc->WAVE = 0x02UL;                          /* single slope PWM: output high from 0 to the compare value */
    LibPWM_Sync(tcc);
    tcc->PER = counts - 1U;
    LibPWM_Sync(tcc);
    tcc->CTRLA |= 0x02UL; /* enable */
    LibPWM_Sync(tcc);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_InitChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, LibPWM_PinsPack_t PinsPack)
{
    const LibPWM_Pin_t *p;
    Lib_Tcc_t          *tcc;

    if(!TIM_Data->Valid || (TIM_Data->TIM > TIM3))
        return LibPWM_Result_TimerNotValid;
    if(Channel > LibPWM_Channel_4)
        return LibPWM_Result_ChannelNotValid;
    p = &LibPWM_Pins[TIM_Data->TIM][Channel];
    if(p->pmux == 0)
        return LibPWM_Result_ChannelNotValid;
    if(PinsPack != LibPWM_PinsPack_1)
        return LibPWM_Result_PinNotValid;
    tcc = LibPWM_Tcc[TIM_Data->TIM];

    tcc->CC[Channel] = 0; /* 0 % */
    LibPWM_Sync(tcc);
    LibGPIO_InitPeripheral(p->group, p->pin, p->pmux);
    TIM_Data->CH_Init |= (uint8_t)(1U << Channel);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_SetChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, uint32_t Pulse)
{
    Lib_Tcc_t *tcc;

    if(!TIM_Data->Valid || (TIM_Data->TIM > TIM3))
        return LibPWM_Result_TimerNotValid;
    if((Channel > LibPWM_Channel_4) || (LibPWM_Pins[TIM_Data->TIM][Channel].pmux == 0))
        return LibPWM_Result_ChannelNotValid;
    tcc = LibPWM_Tcc[TIM_Data->TIM];

    /* compare value 0 = always low; a value above the top of the counter = always high */
    if(Pulse > TIM_Data->Period)
        Pulse = TIM_Data->Period;

    /* the buffer register is copied to the compare register at the end of the period: no glitch */
    LibPWM_Sync(tcc);
    tcc->CCBUF[Channel] = Pulse;

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_SetChannelPercent(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, int32_t percent)
{
    uint32_t pulse;

    if(percent <= 0)
        pulse = 0;
    else if(percent >= 100)
        pulse = TIM_Data->Period;
    else
        pulse = (TIM_Data->Period / 100U) * (uint32_t)percent + ((TIM_Data->Period % 100U) * (uint32_t)percent) / 100U;

    return LibPWM_SetChannel(TIM_Data, Channel, pulse);
}
