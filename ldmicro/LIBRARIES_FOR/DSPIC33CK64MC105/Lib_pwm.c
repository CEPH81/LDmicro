/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_pwm.h"

typedef struct {
    volatile uint16_t *con1l, *con2h, *prl, *ra, *rb, *tmrl;
    uint8_t            rp; /* output pin (RPn) */
} LibPWM_Sccp_t;

static const LibPWM_Sccp_t LibPWM_Sccp[4] = {
    {&CCP1CON1L, &CCP1CON2H, &CCP1PRL, &CCP1RA, &CCP1RB, &CCP1TMRL, 39}, /* RB7 */
    {&CCP2CON1L, &CCP2CON2H, &CCP2PRL, &CCP2RA, &CCP2RB, &CCP2TMRL, 40}, /* RB8 */
    {&CCP3CON1L, &CCP3CON2H, &CCP3PRL, &CCP3RA, &CCP3RB, &CCP3TMRL, 41}, /* RB9 */
    {&CCP4CON1L, &CCP4CON2H, &CCP4PRL, &CCP4RA, &CCP4RB, &CCP4TMRL, 74}, /* RD10 */
};

/* CCPxCON1L */
#define CCP_ON 0x8000U
#define CCP_MOD_PWM 0x0005U /* dual edge compare, buffered: the output rises at RA and falls at RB */
/* CCPxCON2H */
#define CCP_OCAEN 0x0100U

LibPWM_Result_t LibPWM_InitTimer(uint8_t TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency)
{
    /* Prescaler codes 0..3 = 1:1, 1:4, 1:16, 1:64 */
    static const uint8_t shift[4] = {0, 2, 4, 6};
    const LibPWM_Sccp_t *s;
    uint32_t             frequency, clocks, counts = 0;
    uint8_t              code;

    TIM_Data->Valid = 0;
    if((TIMx < TIM1) || (TIMx > TIM4))
        return LibPWM_Result_TimerNotValid;
    s = &LibPWM_Sccp[TIMx - 1U];
    TIM_Data->TIM = TIMx;

    *s->con1l = 0; /* module off: the output is low */

    TIM_Data->Valid = 1;
    TIM_Data->Period = 0;
    TIM_Data->Frequency = 0;
    TIM_Data->Micros = 0;

    if(!(PWMFrequency >= 1.0))
        return (PWMFrequency > 0.0) ? LibPWM_Result_FrequencyTooLow : LibPWM_Result_Ok; /* <= 0: PWM stopped */
    if(PWMFrequency > (double)(SystemCoreClock / 2UL))
        return LibPWM_Result_FrequencyTooHigh;

    frequency = (uint32_t)PWMFrequency;
    clocks = SystemCoreClock / frequency;
    for(code = 0; code < 4; code++) {
        counts = clocks >> shift[code];
        if(counts <= 65535UL)
            break;
    }
    if(code >= 4)
        return LibPWM_Result_FrequencyTooLow;
    if(counts < 2UL)
        return LibPWM_Result_FrequencyTooHigh;

    TIM_Data->Period = counts;
    TIM_Data->Prescaler = code;
    TIM_Data->Frequency = frequency;
    TIM_Data->Micros = 1000000UL / frequency;

    *s->tmrl = 0;
    *s->prl = (uint16_t)(counts - 1UL);
    *s->ra = 0;
    *s->con2h = CCP_OCAEN; /* output enabled */
    /* clock = peripheral clock, prescaler, 16 bits timer, PWM mode, module on */
    *s->con1l = (uint16_t)(CCP_ON | ((uint16_t)code << 6) | CCP_MOD_PWM);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_InitChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, LibPWM_PinsPack_t PinsPack)
{
    const LibPWM_Sccp_t *s;

    if(!TIM_Data->Valid || (TIM_Data->TIM < TIM1) || (TIM_Data->TIM > TIM4))
        return LibPWM_Result_TimerNotValid;
    if(Channel != LibPWM_Channel_1)
        return LibPWM_Result_ChannelNotValid;
    if(PinsPack != LibPWM_PinsPack_1)
        return LibPWM_Result_PinNotValid;
    s = &LibPWM_Sccp[TIM_Data->TIM - 1U];

    *s->rb = 0; /* 0 % */

    /* digital output pin, given to the SCCP module */
    if(s->rp == 74U) {
#ifdef ANSELD
        ANSELD &= (uint16_t)~0x0400U;
#endif
        TRISD &= (uint16_t)~0x0400U;
    } else {
        ANSELB &= (uint16_t)~(uint16_t)(1U << (s->rp - 32U));
        TRISB &= (uint16_t)~(uint16_t)(1U << (s->rp - 32U));
    }
    LibPPS_Output(s->rp, (uint8_t)(LIB_PPS_OCM1A + (TIM_Data->TIM - 1U)));
    TIM_Data->CH_Init |= 1U;

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_SetChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, uint32_t Pulse)
{
    if(!TIM_Data->Valid || (TIM_Data->TIM < TIM1) || (TIM_Data->TIM > TIM4))
        return LibPWM_Result_TimerNotValid;
    if(Channel != LibPWM_Channel_1)
        return LibPWM_Result_ChannelNotValid;

    /* falling edge at 0 = no pulse; falling edge above the period = always high */
    if(Pulse > TIM_Data->Period)
        Pulse = TIM_Data->Period;
    *LibPWM_Sccp[TIM_Data->TIM - 1U].rb = (uint16_t)Pulse;

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
        pulse = (TIM_Data->Period * (uint32_t)percent) / 100UL;

    return LibPWM_SetChannel(TIM_Data, Channel, pulse);
}
