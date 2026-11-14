/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_pwm.h"

/* Indexes in GTCCR[]: A, B, C (buffer of A), E (buffer of B) */
#define GTCCRC 2
#define GTCCRE 3

/* Port and pin of the outputs A and B of each GPT channel; 0xFF = not available on the board */
static const uint8_t LibPWM_Pins[8][2][2] = {
    {{1, 7}, {1, 6}},        /* GPT0: P107, P106 */
    {{1, 9}, {1, 4}},        /* GPT1: P109, P104 */
    {{1, 3}, {1, 2}},        /* GPT2: P103, P102 */
    {{1, 11}, {1, 12}},      /* GPT3: P111, P112 */
    {{3, 2}, {3, 1}},        /* GPT4: P302, P301 */
    {{4, 9}, {0xFF, 0xFF}},  /* GPT5: P409 */
    {{0xFF, 0xFF}, {0xFF, 0xFF}}, /* GPT6: PLC cycle timer */
    {{3, 4}, {3, 3}},        /* GPT7: P304, P303 */
};

static int LibPWM_ValidTimer(uint32_t TIMx)
{
    return (TIMx <= 7U) && (TIMx != TIM6);
}

LibPWM_Result_t LibPWM_InitTimer(uint32_t TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency)
{
    R_GPT0_Type *gpt;
    uint32_t     freq, tpcs = 0, gtpr;

    TIM_Data->TIM = TIMx;
    TIM_Data->Valid = 0;

    if(!LibPWM_ValidTimer(TIMx))
        return LibPWM_Result_TimerNotValid;
    gpt = LibTimer_Gpt(TIMx);
    TIM_Data->Valid = 1;

    if(PWMFrequency < 1.0) {
        /* PWM stopped: the outputs of this structure are driven low by LibPWM_SetChannelPercent() */
        TIM_Data->Period = 0;
        TIM_Data->Frequency = 0;
        TIM_Data->Micros = 0;
        return LibPWM_Result_FrequencyTooLow;
    }
    if(PWMFrequency > (double)(LIB_PCLKD_HZ / 100U))
        return LibPWM_Result_FrequencyTooHigh; /* less than 100 steps of duty cycle */

    freq = (uint32_t)PWMFrequency;
    gtpr = LibTimer_Divide(TIMx, LIB_PCLKD_HZ / freq, &tpcs);
    if(gtpr == 0)
        return LibPWM_Result_FrequencyTooLow;

    TIM_Data->Prescaler = tpcs;
    TIM_Data->Period = gtpr + 1U;
    TIM_Data->Frequency = freq;
    TIM_Data->Micros = 1000000U / freq;

    gpt->GTWP = 0xA500;         /* registers can be written */
    gpt->GTSTP = (1UL << TIMx); /* stop while the period changes */
    gpt->GTCR = (tpcs << 24);   /* saw-wave PWM mode, prescaler */
    gpt->GTUPSR = 0;
    gpt->GTDNSR = 0;
    gpt->GTPBR = gtpr;
    gpt->GTPR = gtpr;
    /* Compare registers A and B buffered by C and E; the buffers are transferred now */
    gpt->GTBER = 0x550000;
    /* Count up; the duty cycle forcing bits (0 % or 100 %) are kept */
    gpt->GTUDDTYC = (gpt->GTUDDTYC & 0x03030000U) | 3U;
    gpt->GTUDDTYC = (gpt->GTUDDTYC & 0x03030000U) | 1U;
    gpt->GTCLR = (1UL << TIMx);
    gpt->GTSTR = (1UL << TIMx);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_InitChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, LibPWM_PinsPack_t PinsPack)
{
    R_GPT0_Type *gpt;
    uint32_t     TIMx = TIM_Data->TIM;
    uint32_t     gtior;

    (void)PinsPack; /* only one pins pack */

    if(!TIM_Data->Valid || !LibPWM_ValidTimer(TIMx))
        return LibPWM_Result_TimerNotValid;
    if((Channel > LibPWM_Channel_2) || (LibPWM_Pins[TIMx][Channel][0] == 0xFF))
        return LibPWM_Result_ChannelNotValid;
    gpt = LibTimer_Gpt(TIMx);

    gpt->GTWP = 0xA500;

    /* Duty cycle 0 % to begin with: output forced low (OADTY / OBDTY = 10) */
    if(Channel == LibPWM_Channel_1) {
        gpt->GTCCR[GTCCRC] = 0;
        gpt->GTUDDTYC = (gpt->GTUDDTYC & ~0x00030000U) | 0x00020000U;
    } else {
        gpt->GTCCR[GTCCRE] = 0;
        gpt->GTUDDTYC = (gpt->GTUDDTYC & ~0x03000000U) | 0x02000000U;
    }

    /* Output low at the compare match and high at the end of the cycle (01001b), output enabled,
       low when the count is stopped */
    gtior = gpt->GTIOR;
    if(Channel == LibPWM_Channel_1)
        gtior = (gtior & 0xFFFF0000U) | 0x0109U; /* GTIOA, OAE */
    else
        gtior = (gtior & 0x0000FFFFU) | 0x01090000U; /* GTIOB, OBE */
    gpt->GTIOR = gtior;

    LibGPIO_InitPeripheral(LibPWM_Pins[TIMx][Channel][0], LibPWM_Pins[TIMx][Channel][1], LIB_PSEL_GPT);
    TIM_Data->CH_Init |= (uint8_t)(1U << Channel);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_SetChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, uint32_t Pulse)
{
    R_GPT0_Type *gpt;
    uint32_t     TIMx = TIM_Data->TIM;
    uint32_t     duty, shift, compare;

    if(!TIM_Data->Valid || !LibPWM_ValidTimer(TIMx))
        return LibPWM_Result_TimerNotValid;
    if(Channel > LibPWM_Channel_2)
        return LibPWM_Result_ChannelNotValid;
    gpt = LibTimer_Gpt(TIMx);

    /* 0 % and 100 % cannot be obtained with the compare match: the output is forced
       (duty setting 10b = 0 %, 11b = 100 %, 00b = compare match) */
    if((Pulse == 0) || (TIM_Data->Period == 0)) {
        duty = 2;
        compare = 0;
    } else if(Pulse >= TIM_Data->Period) {
        duty = 3;
        compare = TIM_Data->Period - 1U;
    } else {
        duty = 0;
        compare = Pulse - 1U;
    }

    gpt->GTWP = 0xA500;
    shift = (Channel == LibPWM_Channel_1) ? 16U : 24U;
    gpt->GTCCR[(Channel == LibPWM_Channel_1) ? GTCCRC : GTCCRE] = compare; /* transferred at the end of the cycle */
    gpt->GTUDDTYC = (gpt->GTUDDTYC & ~(3UL << shift)) | (duty << shift);

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
        pulse = (TIM_Data->Period / 100U) * (uint32_t)percent; /* no overflow with the 32 bits timers */
    else
        pulse = (TIM_Data->Period * (uint32_t)percent) / 100U;

    return LibPWM_SetChannel(TIM_Data, Channel, pulse);
}
