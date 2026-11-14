/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_usr.h"
#include <util/delay_basic.h>

/* swap bits from higher to lower */
uint16_t swap(uint16_t var)
{
    uint8_t  i;
    uint16_t res = 0;

    for(i = 0; i < 16; i++)
        if(var & (uint16_t)(1U << i))
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

/* _delay_loop_2() takes 4 CPU cycles per iteration: 4 iterations per microsecond at 16 MHz */
#define LIB_LOOPS_PER_US (LIB_F_CPU / 4000000UL)

void delay_us(uint16_t us)
{
    while(us > 10000U) {
        _delay_loop_2((uint16_t)(10000U * LIB_LOOPS_PER_US));
        us = (uint16_t)(us - 10000U);
    }
    if(us)
        _delay_loop_2((uint16_t)(us * LIB_LOOPS_PER_US));
}

void delay_ms(uint16_t ms)
{
    while(ms--)
        _delay_loop_2((uint16_t)(1000U * LIB_LOOPS_PER_US));
}
