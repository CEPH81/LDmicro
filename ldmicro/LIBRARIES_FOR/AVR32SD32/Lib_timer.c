/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_timer.h"

/* Number of periods of TCB0 in one PLC cycle, and periods counted so far */
static volatile uint16_t LibTimer_Periods = 1;
static volatile uint16_t LibTimer_Count = 0;

ISR(TCB0_INT_vect)
{
    if(++LibTimer_Count < LibTimer_Periods) {
        TCB0.INTFLAGS = TCB_CAPT_bm;
        return;
    }
    LibTimer_Count = 0;
    TIM1_Handler();              /* sees the flag set, and clears it */
    TCB0.INTFLAGS = TCB_CAPT_bm; /* in case the handler did not */
}

static void LibTimer_Start(uint32_t clocks)
{
    uint32_t periods, counts;
    uint8_t  clksel = TCB_CLKSEL_DIV1_gc;

    if(clocks < 16UL)
        clocks = 16UL;

    /* one period of the timer: up to 65536 counts of CLK_PER / 2 */
    periods = (clocks + 131071UL) / 131072UL;
    if(periods > 65535UL)
        periods = 65535UL;
    counts = clocks / periods;
    if(counts > 65536UL) {
        clksel = TCB_CLKSEL_DIV2_gc;
        counts /= 2UL;
    }
    if(counts > 65536UL)
        counts = 65536UL;

    TCB0.CTRLA = 0; /* stopped */
    TCB0.INTCTRL = 0;
    LibTimer_Periods = (uint16_t)periods;
    LibTimer_Count = 0;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; /* periodic interrupt */
    TCB0.CCMP = (uint16_t)(counts - 1UL);
    TCB0.CNT = 0;
    TCB0.INTFLAGS = TCB_CAPT_bm;
    TCB0.CTRLA = (uint8_t)(clksel | TCB_ENABLE_bm);
}

void LibTimer_InitUs(uint8_t TIMx, uint32_t micros)
{
    uint32_t mhz = SystemCoreClock / 1000000UL;

    if(TIMx != TIM1)
        return;
    if(micros == 0)
        micros = 1;
    if(micros > (0xFFFFFFFFUL / mhz))
        micros = 0xFFFFFFFFUL / mhz;
    LibTimer_Start(mhz * micros);
}

void LibTimer_Init(uint8_t TIMx, uint16_t prediv, uint32_t period)
{
    uint32_t factor = 4UL * prediv;

    if(TIMx != TIM1)
        return;
    if(factor == 0)
        factor = 1;
    if(period > (0xFFFFFFFFUL / factor))
        period = 0xFFFFFFFFUL / factor;
    LibTimer_Start(factor * period);
}

void LibTimer_Interrupts(uint8_t TIMx, FunctionalState able)
{
    if(TIMx != TIM1)
        return;

    if(able != DISABLE) {
        TCB0.INTFLAGS = TCB_CAPT_bm;
        TCB0.INTCTRL = TCB_CAPT_bm;
        sei(); /* global interrupt enable */
    } else {
        TCB0.INTCTRL = 0;
    }
}
