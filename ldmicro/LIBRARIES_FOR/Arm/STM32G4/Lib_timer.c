/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_timer.h"

int LibTimer_EnableClock(TIM_TypeDef *TIMx)
{
    volatile uint32_t tmp;

    if(TIMx == TIM1)
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if(TIMx == TIM15)
        RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    else if(TIMx == TIM16)
        RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
    else if(TIMx == TIM17)
        RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
    else if(TIMx == TIM8)
        RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    else if(TIMx == TIM2)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    else if(TIMx == TIM3)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;
    else if(TIMx == TIM4)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
    else if(TIMx == TIM6)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
    else if(TIMx == TIM7)
        RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN;
    else
        return 0;
    tmp = RCC->APB1ENR1; /* short delay after enabling the clock */
    (void)tmp;
    return 1;
}

static IRQn_Type LibTimer_Irq(TIM_TypeDef *TIMx)
{
    if(TIMx == TIM2)
        return TIM2_IRQn;
    if(TIMx == TIM3)
        return TIM3_IRQn;
    if(TIMx == TIM4)
        return TIM4_IRQn;
    if(TIMx == TIM6)
        return TIM6_DAC_IRQn;
    if(TIMx == TIM15)
        return TIM1_BRK_TIM15_IRQn;
    if(TIMx == TIM16)
        return TIM1_UP_TIM16_IRQn;
    return TIM7_IRQn;
}

/* ticks = number of timer clock periods (SystemCoreClock) in one period */
static void LibTimer_Start(TIM_TypeDef *TIMx, uint32_t psc, uint32_t arr)
{
    TIMx->CR1 = 0;
    TIMx->PSC = psc;
    TIMx->ARR = arr;
    TIMx->CNT = 0;
    TIMx->EGR = TIM_EGR_UG;  /* load the prescaler */
    TIMx->SR = 0;            /* and forget the update event just generated */
    TIMx->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
}

void LibTimer_InitUs(TIM_TypeDef *TIMx, uint32_t micros)
{
    uint32_t perMicro = SystemCoreClock / 1000000U; /* timer clock periods per microsecond */
    uint32_t psc, arr;

    if(!LibTimer_EnableClock(TIMx))
        return;
    if(perMicro == 0)
        perMicro = 1;
    if(micros == 0)
        micros = 1;

    /* The prescaler counts whole microseconds as far as possible, so that the period is exact */
    psc = perMicro;
    arr = micros;
    while(arr > 65536U) { /* 16 bits counter */
        psc *= 2;
        arr = (arr + 1) / 2;
    }
    if(arr < 2) { /* very short periods: count timer clock periods instead */
        psc = 1;
        arr = micros * perMicro;
        if(arr < 2)
            arr = 2;
    }
    if(psc > 65536U)
        psc = 65536U;

    LibTimer_Start(TIMx, psc - 1, arr - 1);
}

void LibTimer_Init(TIM_TypeDef *TIMx, uint16_t prediv, uint32_t period)
{
    uint32_t psc = 4U * (uint32_t)prediv; /* the timer clock is SystemCoreClock, not SystemCoreClock / 4 */

    if(!LibTimer_EnableClock(TIMx))
        return;
    if(psc == 0)
        psc = 1;
    if(psc > 65536U)
        psc = 65536U;
    if(period == 0)
        period = 1;
    if(period > 65536U)
        period = 65536U;

    LibTimer_Start(TIMx, psc - 1, period - 1);
}

void LibTimer_Interrupts(TIM_TypeDef *TIMx, FunctionalState able)
{
    IRQn_Type irq = LibTimer_Irq(TIMx);

    if(able != DISABLE) {
        TIMx->SR = 0;
        TIMx->DIER |= TIM_DIER_UIE;
        NVIC_EnableIRQ(irq);
    } else {
        TIMx->DIER &= ~TIM_DIER_UIE;
        NVIC_DisableIRQ(irq);
    }
}
