/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_timer.h"

R_GPT0_Type *LibTimer_Gpt(uint32_t TIMx)
{
    static R_GPT0_Type *const gpt[8] = {R_GPT0, R_GPT1, R_GPT2, R_GPT3, R_GPT4, R_GPT5, R_GPT6, R_GPT7};
    uint32_t                  bit;

    if(TIMx > 7U)
        return 0;

    /* Module stop: MSTPD5 for the 32 bits channels (0 and 1), MSTPD6 for the 16 bits ones */
    bit = (TIMx <= 1U) ? (1UL << 5) : (1UL << 6);
    if(R_MSTP->MSTPCRD & bit) {
        R_SYSTEM->PRCR = LIB_PRCR_UNLOCK;
        R_MSTP->MSTPCRD &= ~bit;
        R_SYSTEM->PRCR = LIB_PRCR_LOCK;
    }
    return gpt[TIMx];
}

uint32_t LibTimer_Divide(uint32_t TIMx, uint64_t clocks, uint32_t *tpcs)
{
    uint64_t maxCount = (TIMx <= 1U) ? 0x100000000ULL : 0x10000ULL; /* GPT0 and GPT1 are 32 bits counters */
    uint32_t code;

    for(code = 0; code < 6; code++) {
        uint64_t counts = clocks >> (2U * code); /* /1, /4, /16, /64, /256, /1024 */
        if(counts <= maxCount) {
            *tpcs = code;
            return (counts < 2U) ? 0U : (uint32_t)(counts - 1U); /* value of GTPR */
        }
    }
    return 0;
}

static void LibTimer_Start(uint32_t TIMx, uint32_t tpcs, uint32_t gtpr)
{
    R_GPT0_Type *gpt = LibTimer_Gpt(TIMx);

    if(gpt == 0)
        return;

    gpt->GTWP = 0xA500;         /* registers can be written */
    gpt->GTSTP = (1UL << TIMx); /* stop */
    gpt->GTCR = (tpcs << 24);   /* saw-wave mode; prescaler TPCS[2:0] of the RA4M1 = bits 26..24 */
    gpt->GTUPSR = 0;
    gpt->GTDNSR = 0;
    gpt->GTBER = 0;             /* no buffer operation needed for a plain periodic timer... */
    gpt->GTPBR = gtpr;          /* ...but keep the period buffer coherent */
    gpt->GTPR = gtpr;
    gpt->GTIOR = 0;             /* no output */
    gpt->GTUDDTYC = 3;          /* count up (set, then release the forcing bit) */
    gpt->GTUDDTYC = 1;
    gpt->GTCLR = (1UL << TIMx); /* counter = 0 */
    gpt->GTST = 0;
    gpt->GTSTR = (1UL << TIMx); /* start */
}

void LibTimer_InitUs(uint32_t TIMx, uint32_t micros)
{
    uint64_t clocks;
    uint32_t tpcs = 0, gtpr;

    if(micros == 0)
        micros = 1;
    clocks = (uint64_t)(LIB_PCLKD_HZ / 1000000U) * micros; /* PCLKD is a whole number of MHz */
    gtpr = LibTimer_Divide(TIMx, clocks, &tpcs);
    if(gtpr == 0) { /* too long for the counter: longest period */
        tpcs = 5;
        gtpr = 0xFFFFU;
    }
    LibTimer_Start(TIMx, tpcs, gtpr);
}

void LibTimer_Init(uint32_t TIMx, uint16_t prediv, uint32_t period)
{
    uint64_t clocks = 4ULL * prediv * period; /* in periods of SystemCoreClock = PCLKD */
    uint32_t tpcs = 0, gtpr;

    gtpr = LibTimer_Divide(TIMx, clocks, &tpcs);
    if(gtpr == 0) {
        tpcs = 5;
        gtpr = 0xFFFFU;
    }
    LibTimer_Start(TIMx, tpcs, gtpr);
}

void LibTimer_Interrupts(uint32_t TIMx, FunctionalState able)
{
    if(TIMx != TIM6)
        return; /* only the PLC cycle timer has an interrupt line */

    if(able != DISABLE) {
        R_ICU->IELSR[TIM6_IRQn] = LIB_EVENT_GPT6_OVERFLOW; /* link the event, flag cleared */
        NVIC_ClearPendingIRQ(TIM6_IRQn);
        NVIC_EnableIRQ(TIM6_IRQn);
    } else {
        NVIC_DisableIRQ(TIM6_IRQn);
        R_ICU->IELSR[TIM6_IRQn] = 0;
    }
}

void LibTimer_ClearInterrupt(uint32_t TIMx)
{
    (void)TIMx;
    /* The interrupt request flag of the line must be cleared by the handler */
    R_ICU->IELSR[TIM6_IRQn] &= ~LIB_IELSR_IR;
    (void)R_ICU->IELSR[TIM6_IRQn];
}
