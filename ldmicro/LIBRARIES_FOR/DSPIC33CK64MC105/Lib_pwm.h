/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_PWM_H
#define LIB_PWM_H

/*
    PWM outputs: one SCCP module (16 bits timer) for each output, so each output has its
    own frequency. The outputs go to the pins through the Peripheral Pin Select.

    TIMER           CHANNEL 1
    TIM1 = SCCP1    RB7  (RP39)
    TIM2 = SCCP2    RB8  (RP40)
    TIM3 = SCCP3    RB9  (RP41)
    TIM4 = SCCP4    RD10 (RP74): LED of the board, see NB

    TIM0 (Timer1) is the timer of the PLC cycle.

    NB: the LED of the Curiosity Nano board lights when RD10 is low: the brightness is the
    opposite of the duty cycle.
*/

#include "Lib_mcu.h"

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
    uint8_t  TIM;       /* Timer used: 1..4 = SCCP1..SCCP4 */
    uint8_t  Valid;     /* 1 when LibPWM_InitTimer() accepted the timer */
    uint32_t Period;    /* Number of timer counts in one period; 0 = stopped */
    uint8_t  Prescaler; /* Prescaler code of the SCCP timer */
    uint32_t Frequency; /* PWM frequency in Hz */
    uint32_t Micros;    /* Period in microseconds */
    uint8_t  CH_Init;   /* Bit mask of the channels already initialised */
} LibPWM_TIM_t;

typedef enum {
    LibPWM_Channel_1 = 0x00, /* the output of the SCCP module */
    LibPWM_Channel_2 = 0x01, /* not available */
    LibPWM_Channel_3 = 0x02, /* not available */
    LibPWM_Channel_4 = 0x03  /* not available */
} LibPWM_Channel_t;

typedef enum {
    LibPWM_PinsPack_1 = 0x00,
    LibPWM_PinsPack_2,
    LibPWM_PinsPack_3
} LibPWM_PinsPack_t;

/* Sets the frequency of the timer; a frequency <= 0 stops the PWM (outputs low) */
LibPWM_Result_t LibPWM_InitTimer(uint8_t TIMx, LibPWM_TIM_t *TIM_Data, double PWMFrequency);

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
