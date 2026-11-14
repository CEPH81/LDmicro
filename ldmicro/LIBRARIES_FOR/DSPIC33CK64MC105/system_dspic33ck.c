/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_mcu.h"

/*
 * Configuration bits. The names are those of the dsPIC33CK64MC105 header of the compiler.
 *  - internal FRC oscillator (8 MHz), no clock switching: FP = FCY = 4 MHz
 *  - watchdog timer off (it can be turned on by software)
 *  - JTAG off: RB7, RB10, RB11 and RB12 are ordinary pins
 *  - debugger of the Curiosity Nano board on PGC3/PGD3
 *  - I2C1 on its alternate pins ASCL1/ASDA1 (RC9/RC8), as wired on the board
 */
#pragma config FNOSC = FRC
#pragma config IESO = OFF
#pragma config POSCMD = NONE
#pragma config OSCIOFNC = ON
#pragma config FCKSM = CSDCMD
#pragma config FWDTEN = ON_SW
#pragma config ICS = PGD3
#pragma config JTAGEN = OFF
#pragma config ALTI2C1 = ON

uint32_t SystemCoreClock = LIB_FCY;

Lib_Handle_t Lib_Uart1 = {1}, Lib_Spi1 = {1}, Lib_I2c1 = {1}, Lib_Adc = {0};

void SystemInit(void)
{
    /* the clock is set by the configuration bits */
    SystemCoreClock = LIB_FCY;
}

/* The registers RPOR0, RPOR1... hold two pins each, from RP32: RPORn = RP(32 + 2n) and RP(33 + 2n) */
void LibPPS_Output(uint8_t rp, uint8_t function)
{
    volatile uint16_t *rpor;

    if(rp < 32U)
        return;
    rpor = &RPOR0 + ((rp - 32U) >> 1);

    __builtin_write_RPCON(0x0000); /* unlock the pin selection */
    if(rp & 1U)
        *rpor = (uint16_t)((*rpor & 0x00FFU) | ((uint16_t)function << 8));
    else
        *rpor = (uint16_t)((*rpor & 0xFF00U) | function);
    __builtin_write_RPCON(0x0800); /* lock */
}
