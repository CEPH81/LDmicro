/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_mcu.h"

uint32_t SystemCoreClock = 4000000UL; /* OSCHF at 4 MHz after reset */

/*
 * The AVR SD is a functional safety MCU: its error controller (ERRCTRL) cannot be turned
 * off and, with the reset settings, any error it detects floats all the I/O pins and resets
 * the MCU. As the data sheet advises for the start of a development, every error source is
 * given the severity level "notification": the errors are only recorded in the flags.
 * A safety application must set the levels according to its own safety analysis.
 */
static void SystemErrorController(void)
{
    volatile uint8_t *esc;

    if(ERRCTRL.ESF)
        ERRCTRL.ESF = 0xFFFFFFFFUL; /* clear the error status flags */

    _PROTECTED_WRITE(ERRCTRL.CTRLA, ERRCTRL_STATE_CONFIG_gc);
    for(esc = &ERRCTRL.ESCVREGFAIL; esc <= &ERRCTRL.ESCEVSYS1; esc++)
        *esc = ERRCTRL_ERRLVL_NOTIFICATION_gc;
    _PROTECTED_WRITE(ERRCTRL.CTRLA, ERRCTRL_STATE_NORMAL_gc);
}

void SystemInit(void)
{
    SystemErrorController();

    /* Internal high frequency oscillator at 16 MHz, main clock not divided */
    _PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_16M_gc);
    SystemCoreClock = LIB_F_CPU;
}
