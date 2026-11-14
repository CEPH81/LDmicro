/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_gpio.h"

/* Output pins configured by the PLC program, for each port */
static uint16_t LibGPIO_Outputs[6];

static int LibGPIO_PortNumber(GPIO_TypeDef *GPIOx)
{
    if(GPIOx == R_PORT0)
        return 0;
    if(GPIOx == R_PORT1)
        return 1;
    if(GPIOx == R_PORT2)
        return 2;
    if(GPIOx == R_PORT3)
        return 3;
    if(GPIOx == R_PORT4)
        return 4;
    if(GPIOx == R_PORT5)
        return 5;
    return -1;
}

static void LibGPIO_Unlock(void)
{
    R_PMISC->PWPR = 0;    /* B0WI = 0: PFSWE can be written */
    R_PMISC->PWPR = 0x40; /* PFSWE = 1: the PmnPFS registers can be written */
}

void LibGPIO_Conf(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_Type, uint32_t GPIO_Speed)
{
    int      port = LibGPIO_PortNumber(GPIOx);
    uint32_t pin, pfs;

    (void)GPIO_Speed;

    if(port < 0)
        return;
    LibGPIO_Unlock();

    for(pin = 0; pin < 16; pin++) {
        if((GPIO_Pin & (1U << pin)) == 0)
            continue;

        /* Writing the whole register also returns the pin to the general I/O function
           (PMR = 0, ASEL = 0, ISEL = 0) whatever its previous use */
        if(GPIO_Mode == GPIO_Mode_OUT) {
            pfs = R_PFS->PORT[port].PIN[pin].PmnPFS & 0x01U; /* keep the output level (PODR) */
            pfs |= 0x04U;                                     /* PDR = 1: output */
            if(GPIO_Type == GPIO_OType_OD)
                pfs |= 0x40U; /* NCODR = 1: N-channel open drain */
            LibGPIO_Outputs[port] |= (uint16_t)(1U << pin);
        } else {
            pfs = 0; /* PDR = 0: input */
            if(GPIO_Type == GPIO_PuPd_UP)
                pfs |= 0x10U; /* PCR = 1: pull-up */
            LibGPIO_Outputs[port] &= (uint16_t)~(1U << pin);
        }
        R_PFS->PORT[port].PIN[pin].PmnPFS = pfs;
    }
}

void LibGPIO_Write(GPIO_TypeDef *GPIOx, uint16_t value)
{
    int      port = LibGPIO_PortNumber(GPIOx);
    uint16_t mask;

    if(port < 0)
        return;
    mask = LibGPIO_Outputs[port];
    GPIOx->POSR = (uint16_t)(value & mask);
    GPIOx->PORR = (uint16_t)(~value & mask);
}

void LibGPIO_InitPeripheral(uint32_t port, uint32_t pin, uint32_t psel)
{
    LibGPIO_Unlock();
    /* The function is selected first, then the pin is switched to the peripheral (PMR, bit 16) */
    R_PFS->PORT[port].PIN[pin].PmnPFS = (psel << 24);
    R_PFS->PORT[port].PIN[pin].PmnPFS = (psel << 24) | (1UL << 16);
    if(port < 6)
        LibGPIO_Outputs[port] &= (uint16_t)~(1U << pin);
}

void LibGPIO_InitAnalog(uint32_t port, uint32_t pin)
{
    LibGPIO_Unlock();
    R_PFS->PORT[port].PIN[pin].PmnPFS = (1UL << 15); /* ASEL = 1, input, no pull-up */
    if(port < 6)
        LibGPIO_Outputs[port] &= (uint16_t)~(1U << pin);
}
