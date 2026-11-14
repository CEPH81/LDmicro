/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_TIMER_H
#define LIB_TIMER_H

#include "Lib_mcu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PLC cycle timer: TIM4 = TC0 (16 bits). The other timers (TCC0..TCC3) are used by Lib_pwm.
 * Longest period: about 4 s at 16 MHz.
 */
void LibTimer_InitUs(uint32_t TIMx, uint32_t micros);
/* Same arguments as the STM32F4 library: period of (4 * prediv * period) CPU clocks */
void LibTimer_Init(uint32_t TIMx, uint16_t prediv, uint32_t period);
void LibTimer_Interrupts(uint32_t TIMx, FunctionalState able);

/* Finds the prescaler of a TC or TCC: returns the count for the period (0 if out of range)
   and the prescaler code; maxCount = 0x10000 or 0x1000000 */
uint32_t LibTimer_Divide(uint32_t clocks, uint32_t maxCount, uint32_t *prescaler);

#ifdef __cplusplus
}
#endif

#endif
