/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef SYSTEM_RENESAS_ARM_H
#define SYSTEM_RENESAS_ARM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t SystemCoreClock; /* ICLK, in Hz */

void SystemInit(void);
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif
