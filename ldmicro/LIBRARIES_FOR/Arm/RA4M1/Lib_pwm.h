/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_PWM_H
#define LIB_PWM_H

/*
    PWM outputs of the Arduino Nano R4 (GPT timers; channel 1 = output GTIOCnA, channel 2 = GTIOCnB):

    TIMER   CHANNEL 1           CHANNEL 2
    TIM0    P107 (D7)           P106 (D6)
    TIM1    P109 (D11)          P104 (D3)
    TIM2    P103 (D4)           P102 (D5)
    TIM3    P111 (D13)          P112 (D10)
    TIM4    P302 (D1)           P301 (D0)
    TIM5    P409 (red LED)      -
    TIM7    P304 (D8)           P303 (D9)

    TIM6 is the timer of the PLC cycle. Both channels of one timer share the same frequency.
    TIM0 and TIM1 are 32 bits counters, the others 16 bits.
*/

#include "Lib_mcu.h"
#include "Lib_gpio.h"
#include "Lib_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LibPWM_Result_Ok = 0,           /* Everything OK */
    LibPWM_Result_FrequencyTooHigh, /* Frequency too high for the timer */
    LibPWM_Result_FrequencyTooLow,  /* Frequency too low for the timer */
    LibPWM_Result_PulseTooHigh,     /* Pulse larger than the timer period */
    LibPWM_Result_TimerNotValid,    /* Timer without PWM output */
    LibPWM_Result_ChannelNotValid,  /* Channel not available on this timer */
    LibPWM_Result_PinNotValid       /* Pins pack not available */
} LibPWM_Result_t;

typedef struct {
    uint32_t TIM;       /* Timer used (number of the GPT channel) */
    uint8_t  Valid;     /* 1 when LibPWM_InitTimer() accepted the timer */
    uint32_t Period;    /* Number of timer counts in one period; 0 = stopped */
    uint32_t Prescaler; /* Prescaler code (TPCS) */
    uint32_t Frequency; /* PWM frequency in Hz */
    uint32_t Micros;    /* Period in microseconds */
    uint8_t  CH_Init;   /* Bit mask of the channels already initialised */
} LibPWM_TIM_t;

typedef enum {
    LibPWM_Channel_1 = 0x00, /* GTIOCnA */
    LibPWM_Channel_2 = 0x01, /* GTIOCnB */
    LibPWM_Channel_3 = 0x02, /* not available */
    LibPWM_Channel_4 = 0x03  /* not available */
} LibPWM_Channel_t;

typedef enum {
    LibPWM_PinsPack_1 = 0x00,
    LibPWM_PinsPack_2,
    LibPWM_PinsPack_3
} LibPWM_PinsPack_t;

/* Sets the frequency of the timer; a frequency <= 0 stops the PWM (outputs low) */
LibPWM_Result_t LibPWM_InitTimer(uint32_t TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency);

/* Configures the pin and the output of one channel */
LibPWM_Result_t LibPWM_InitChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, LibPWM_PinsPack_t PinsPack);

/* Pulse length in timer counts */
LibPWM_Result_t LibPWM_SetChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, uint32_t Pulse);

/* Duty cycle from 0 to 100 % */
LibPWM_Result_t LibPWM_SetChannelPercent(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, int32_t percent);

#ifdef __cplusplus
}
#endif

#endif
