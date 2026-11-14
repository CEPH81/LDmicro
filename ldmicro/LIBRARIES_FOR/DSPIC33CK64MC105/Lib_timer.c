/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_timer.h"

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    TIM0_Handler();   /* sees the flag set, and clears it */
    IFS0bits.T1IF = 0; /* in case the handler did not */
}

static void LibTimer_Start(uint32_t clocks)
{
    /* Prescaler codes 0..3 = 1:1, 1:8, 1:64, 1:256 */
    static const uint8_t shift[4] = {0, 3, 6, 8};
    uint8_t              code;
    uint32_t             counts = 0;

    for(code = 0; code < 4; code++) {
        counts = clocks >> shift[code];
        if(counts <= 65536UL)
            break;
    }
    if(code >= 4) { /* too long for the timer: longest period */
        code = 3;
        counts = 65536UL;
    }
    if(counts < 2UL)
        counts = 2UL;

    T1CON = 0; /* timer off, clock = peripheral clock */
    TMR1 = 0;
    PR1 = (uint16_t)(counts - 1UL);
    T1CONbits.TCKPS = code;
    IFS0bits.T1IF = 0;
    T1CONbits.TON = 1;
}

void LibTimer_InitUs(uint8_t TIMx, uint32_t micros)
{
    uint32_t mhz = SystemCoreClock / 1000000UL;

    if(TIMx != TIM0)
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

    if(TIMx != TIM0)
        return;
    if(factor == 0)
        factor = 1;
    if(period > (0xFFFFFFFFUL / factor))
        period = 0xFFFFFFFFUL / factor;
    LibTimer_Start(factor * period);
}

void LibTimer_Interrupts(uint8_t TIMx, FunctionalState able)
{
    if(TIMx != TIM0)
        return;

    if(able != DISABLE) {
        IFS0bits.T1IF = 0;
        IEC0bits.T1IE = 1;
    } else {
        IEC0bits.T1IE = 0;
    }
}
