/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
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
 * TIMx = TIM6 (GPT6, 16 bits): its overflow is linked to the interrupt line 0 (TIM6_Handler).
 */
void LibTimer_InitUs(uint32_t TIMx, uint32_t micros);

/* Same calling convention as the STM32F4 library: frequency = (SystemCoreClock / 4) / (prediv * period) */
void LibTimer_Init(uint32_t TIMx, uint16_t prediv, uint32_t period);

/* Enables or disables the overflow interrupt of the PLC cycle timer */
void LibTimer_Interrupts(uint32_t TIMx, FunctionalState able);

/* To be called in the interrupt routine (TIM_ClearITPendingBit) */
void LibTimer_ClearInterrupt(uint32_t TIMx);

/* Registers of a GPT channel (0 to 7), 0 if the channel does not exist; the module is started */
R_GPT0_Type *LibTimer_Gpt(uint32_t TIMx);

/* Finds the prescaler: returns the TPCS code (0..5 = /1, /4, /16, /64, /256, /1024) and the number
   of counts of one period for a frequency given in clock periods of PCLKD; 0 counts if not possible */
uint32_t LibTimer_Divide(uint32_t TIMx, uint64_t clocks, uint32_t *tpcs);

#ifdef __cplusplus
}
#endif

#endif
