/**
 * LDmicro library for STM32L4 (STM32L432), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_PWM_H
#define LIB_PWM_H

/*
    PWM outputs (there is only one pins pack):

    TIMER   CHANNEL 1   CHANNEL 2   CHANNEL 3   CHANNEL 4
    TIM1    PA8         PA9         PA10        PA11
    TIM2    PA0         PA1         PA2         PA3
    TIM15   PA2         PA3         -           -

    All the channels of one timer share the same frequency.
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
    TIM_TypeDef *TIM;       /* Timer used */
    uint32_t     Period;    /* Number of timer counts in one period (auto-reload value + 1); 0 = stopped */
    uint32_t     Prescaler; /* Prescaler value (PSC register) */
    uint32_t     Frequency; /* PWM frequency in Hz */
    uint32_t     Micros;    /* Period in microseconds */
    uint8_t      CH_Init;   /* Bit mask of the channels already initialised */
} LibPWM_TIM_t;

typedef enum {
    LibPWM_Channel_1 = 0x00,
    LibPWM_Channel_2 = 0x01,
    LibPWM_Channel_3 = 0x02,
    LibPWM_Channel_4 = 0x03
} LibPWM_Channel_t;

typedef enum {
    LibPWM_PinsPack_1 = 0x00,
    LibPWM_PinsPack_2,
    LibPWM_PinsPack_3
} LibPWM_PinsPack_t;

/* Sets the frequency of the timer; a frequency <= 0 stops the PWM (outputs low) */
LibPWM_Result_t LibPWM_InitTimer(TIM_TypeDef *TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency);

/* Configures the pin and the output compare of one channel */
LibPWM_Result_t LibPWM_InitChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, LibPWM_PinsPack_t PinsPack);

/* Pulse length in timer counts */
LibPWM_Result_t LibPWM_SetChannel(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, uint32_t Pulse);

/* Duty cycle from 0 to 100 % */
LibPWM_Result_t LibPWM_SetChannelPercent(LibPWM_TIM_t *TIM_Data, LibPWM_Channel_t Channel, int32_t percent);

#ifdef __cplusplus
}
#endif

#endif
