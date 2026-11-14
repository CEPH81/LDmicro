/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_pwm.h"

LibPWM_Result_t LibPWM_InitTimer(uint8_t TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency)
{
    /* Clock selections 0..7 = CLK_PER / 1, 2, 4, 8, 16, 64, 256, 1024 */
    static const uint8_t shift[8] = {0, 1, 2, 3, 4, 6, 8, 10};
    uint32_t             frequency, clocks, counts = 0;
    uint8_t              clksel;

    TIM_Data->Valid = 0;
    if(TIMx != TIM0)
        return LibPWM_Result_TimerNotValid;
    TIM_Data->TIM = TIMx;

    /* stop the counter; the outputs stay as they are until the timer runs again */
    TCA0.SINGLE.CTRLA = 0;

    TIM_Data->Valid = 1;
    TIM_Data->Period = 0;
    TIM_Data->Frequency = 0;
    TIM_Data->Micros = 0;

    if(!(PWMFrequency >= 1.0)) {
        /* PWM stopped: outputs low */
        TCA0.SINGLE.CMP1 = 0;
        TCA0.SINGLE.CMP2 = 0;
        return (PWMFrequency > 0.0) ? LibPWM_Result_FrequencyTooLow : LibPWM_Result_Ok;
    }
    if(PWMFrequency > (double)(SystemCoreClock / 2UL))
        return LibPWM_Result_FrequencyTooHigh;

    frequency = (uint32_t)PWMFrequency;
    clocks = SystemCoreClock / frequency;
    for(clksel = 0; clksel < 8; clksel++) {
        counts = clocks >> shift[clksel];
        if(counts <= 65535UL)
            break;
    }
    if(clksel >= 8)
        return LibPWM_Result_FrequencyTooLow;
    if(counts < 2UL)
        return LibPWM_Result_FrequencyTooHigh;

    TIM_Data->Period = counts;
    TIM_Data->Prescaler = clksel;
    TIM_Data->Frequency = frequency;
    TIM_Data->Micros = 1000000UL / frequency;

    /* outputs of TCA0 on PORTD */
    PORTMUX.TCAROUTEA = (uint8_t)((PORTMUX.TCAROUTEA & (uint8_t)~PORTMUX_TCA0_gm) | PORTMUX_TCA0_PORTD_gc);

    /* single slope PWM; the compare outputs already enabled are kept */
    TCA0.SINGLE.CTRLB = (uint8_t)((TCA0.SINGLE.CTRLB & (TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_CMP2EN_bm)) | TCA_SINGLE_WGMODE_SINGLESLOPE_gc);
    TCA0.SINGLE.PER = (uint16_t)(counts - 1UL);
    TCA0.SINGLE.CNT = 0;
    TCA0.SINGLE.CTRLA = (uint8_t)((uint8_t)(clksel << TCA_SINGLE_CLKSEL_gp) | TCA_SINGLE_ENABLE_bm);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_InitChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, LibPWM_PinsPack_t PinsPack)
{
    if(!TIM_Data->Valid || (TIM_Data->TIM != TIM0))
        return LibPWM_Result_TimerNotValid;
    if((Channel != LibPWM_Channel_2) && (Channel != LibPWM_Channel_3))
        return LibPWM_Result_ChannelNotValid;
    if(PinsPack != LibPWM_PinsPack_1)
        return LibPWM_Result_PinNotValid;

    if(Channel == LibPWM_Channel_2) {
        TCA0.SINGLE.CMP1 = 0; /* 0 % */
        TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;
        PORTD.DIRSET = PIN1_bm;
    } else {
        TCA0.SINGLE.CMP2 = 0;
        TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
        PORTD.DIRSET = PIN2_bm;
    }
    TIM_Data->CH_Init |= (uint8_t)(1U << Channel);

    return LibPWM_Result_Ok;
}

LibPWM_Result_t LibPWM_SetChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, uint32_t Pulse)
{
    if(!TIM_Data->Valid || (TIM_Data->TIM != TIM0))
        return LibPWM_Result_TimerNotValid;
    if((Channel != LibPWM_Channel_2) && (Channel != LibPWM_Channel_3))
        return LibPWM_Result_ChannelNotValid;

    /* compare value 0 = always low; a value above the period register = always high */
    if(Pulse > TIM_Data->Period)
        Pulse = TIM_Data->Period;

    /* the buffer register is copied to the compare register at the end of the period: no glitch */
    if(Channel == LibPWM_Channel_2)
        TCA0.SINGLE.CMP1BUF = (uint16_t)Pulse;
    else
        TCA0.SINGLE.CMP2BUF = (uint16_t)Pulse;

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
