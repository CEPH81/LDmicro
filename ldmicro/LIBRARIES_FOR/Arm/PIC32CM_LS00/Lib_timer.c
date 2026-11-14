/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_timer.h"

#define LIB_TIMER_TIMEOUT 100000U

uint32_t LibTimer_Divide(uint32_t clocks, uint32_t maxCount, uint32_t *prescaler)
{
    /* Prescaler codes 0..7 = /1, /2, /4, /8, /16, /64, /256, /1024 */
    static const uint8_t shift[8] = {0, 1, 2, 3, 4, 6, 8, 10};
    uint32_t             code;

    for(code = 0; code < 8; code++) {
        uint32_t counts = clocks >> shift[code];
        if(counts <= maxCount) {
            *prescaler = code;
            return (counts < 2U) ? 0U : counts;
        }
    }
    return 0;
}

static void LibTimer_Sync(void)
{
    uint32_t n;

    for(n = 0; LIB_TC0->SYNCBUSY && (n < LIB_TIMER_TIMEOUT); n++)
        ;
}

static void LibTimer_Start(uint32_t clocks)
{
    uint32_t prescaler = 0, counts;

    counts = LibTimer_Divide(clocks, 0x10000U, &prescaler);
    if(counts == 0) {
        if(clocks < 2U) {
            prescaler = 0;
            counts = 2;
        } else { /* too long for the counter: longest period */
            prescaler = 7;
            counts = 0x10000U;
        }
    }

    LibClock_Enable(LIB_GCLK_TC0);

    LIB_TC0->CTRLA = 0x01U; /* software reset */
    LibTimer_Sync();
    /* 16 bits counter, prescaler, counter reloaded on the prescaler clock */
    LIB_TC0->CTRLA = (prescaler << 8) | (1UL << 4);
    LIB_TC0->WAVE = 0x01U; /* match frequency: the period is CC0 */
    LIB_TC0->CC[0] = (uint16_t)(counts - 1U);
    LibTimer_Sync();
    LIB_TC0->INTFLAG = 0x33U;
    LIB_TC0->CTRLA |= 0x02U; /* enable */
    LibTimer_Sync();
}

void LibTimer_InitUs(uint32_t TIMx, uint32_t micros)
{
    uint32_t mhz = SystemCoreClock / 1000000U;

    if(TIMx != TIM4)
        return;
    if(micros == 0)
        micros = 1;
    if(micros > (0xFFFFFFFFUL / mhz))
        micros = 0xFFFFFFFFUL / mhz;
    LibTimer_Start(mhz * micros);
}

void LibTimer_Init(uint32_t TIMx, uint16_t prediv, uint32_t period)
{
    uint32_t factor = 4UL * prediv;

    if(TIMx != TIM4)
        return;
    if(factor == 0)
        factor = 1;
    if(period > (0xFFFFFFFFUL / factor))
        period = 0xFFFFFFFFUL / factor;
    LibTimer_Start(factor * period);
}

void LibTimer_Interrupts(uint32_t TIMx, FunctionalState able)
{
    if(TIMx != TIM4)
        return;

    if(able != DISABLE) {
        LIB_TC0->INTFLAG = 0x01U;
        LIB_TC0->INTENSET = 0x01U; /* overflow */
        NVIC_ClearPendingIRQ(TC0_IRQn);
        NVIC_EnableIRQ(TC0_IRQn);
    } else {
        NVIC_DisableIRQ(TC0_IRQn);
        LIB_TC0->INTENCLR = 0x01U;
    }
}
