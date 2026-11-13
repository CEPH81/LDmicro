/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_TIMER_H
#define LIB_TIMER_H

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Periodic timer with a period given in microseconds (PLC cycle time).
 * The dividers are computed from SystemCoreClock.
 * TIMx = TIM6 or TIM7 (basic timers), TIM2, TIM3, TIM4, TIM15 or TIM16
 */
void LibTimer_InitUs(TIM_TypeDef *TIMx, uint32_t micros);

/**
 * Same calling convention as the STM32F4 library of LDmicro:
 * frequency = (SystemCoreClock / 4) / (prediv * period)
 */
void LibTimer_Init(TIM_TypeDef *TIMx, uint16_t prediv, uint32_t period);

/* Enables or disables the update interrupt of the timer (and its NVIC line) */
void LibTimer_Interrupts(TIM_TypeDef *TIMx, FunctionalState able);

/* Enables the clock of a timer; returns 0 if the timer does not exist */
int LibTimer_EnableClock(TIM_TypeDef *TIMx);

#ifdef __cplusplus
}
#endif

#endif
