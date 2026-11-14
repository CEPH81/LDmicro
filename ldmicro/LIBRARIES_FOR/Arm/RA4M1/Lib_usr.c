/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_usr.h"

/* swap bits from higher to lower */
uint16_t swap(uint16_t var)
{
    int      i;
    uint16_t res = 0;

    for(i = 0; i < 16; i++)
        if(var & (1U << i))
            res |= (uint16_t)(1U << (15 - i));
    return res;
}

/* take the opposite */
int16_t opposite(int16_t var)
{
    return (int16_t)(-var);
}

/* convert BCD to DEC */
uint8_t bcd2bin(uint8_t var)
{
    return (uint8_t)(10 * (var / 16) + (var % 16));
}

/* convert DEC to BCD */
uint8_t bin2bcd(uint8_t var)
{
    return (uint8_t)(16 * (var / 10) + (var % 10));
}

static void delay_cycles(uint32_t cycles)
{
    uint32_t start = DWT->CYCCNT;

    if((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0) { /* SystemInit() not called: approximate loop */
        volatile uint32_t d = cycles / 8U;
        while(d--)
            ;
        return;
    }
    while((DWT->CYCCNT - start) < cycles)
        ;
}

void delay_us(uint16_t us)
{
    delay_cycles((SystemCoreClock / 1000000U) * us);
}

void delay_ms(uint16_t ms)
{
    while(ms--)
        delay_cycles(SystemCoreClock / 1000U);
}

void _sbrk()
{
}
