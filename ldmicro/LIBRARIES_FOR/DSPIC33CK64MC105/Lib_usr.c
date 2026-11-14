/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_usr.h"
#include <libpic30.h>

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

/* __delay32() of the compiler library waits a number of instruction cycles (FCY) */
void delay_us(uint16_t us)
{
    if(us)
        __delay32((LIB_FCY / 1000000UL) * us);
}

void delay_ms(uint16_t ms)
{
    while(ms--)
        __delay32(LIB_FCY / 1000UL);
}
