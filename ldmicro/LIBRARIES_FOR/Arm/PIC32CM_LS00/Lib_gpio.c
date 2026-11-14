/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_gpio.h"

const GPIO_TypeDef LibGPIO_Ports[4] = {{0, 0}, {1, 0}, {0, 16}, {1, 16}};

/* Output pins configured by the PLC program, for each group */
static uint32_t LibGPIO_Outputs[2];

static void LibGPIO_ConfGroup(Lib_PortGroup_t *port, uint32_t pins, uint32_t GPIO_Mode, uint32_t GPIO_Type)
{
    uint32_t pin;

    for(pin = 0; pin < 32; pin++) {
        if((pins & (1UL << pin)) == 0)
            continue;
        /* general purpose I/O (multiplexer off), input buffer on so that the pin can be read back */
        if(GPIO_Mode == GPIO_Mode_OUT) {
            port->PINCFG[pin] = LIB_PINCFG_INEN;
        } else if(GPIO_Type == GPIO_PuPd_UP) {
            port->OUTSET = 1UL << pin; /* the pull direction is given by the output value */
            port->PINCFG[pin] = LIB_PINCFG_INEN | LIB_PINCFG_PULLEN;
        } else if(GPIO_Type == GPIO_PuPd_DOWN) {
            port->OUTCLR = 1UL << pin;
            port->PINCFG[pin] = LIB_PINCFG_INEN | LIB_PINCFG_PULLEN;
        } else {
            port->PINCFG[pin] = LIB_PINCFG_INEN;
        }
    }
    if(GPIO_Mode == GPIO_Mode_OUT)
        port->DIRSET = pins;
    else
        port->DIRCLR = pins;
}

void LibGPIO_Conf(const GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_Type, uint32_t GPIO_Speed)
{
    uint32_t pins = (uint32_t)GPIO_Pin << GPIOx->shift;

    (void)GPIO_Speed;

    if(GPIO_Mode == GPIO_Mode_OUT)
        LibGPIO_Outputs[GPIOx->group] |= pins;
    else
        LibGPIO_Outputs[GPIOx->group] &= ~pins;

    LibGPIO_ConfGroup(&LIB_PORT_SEC[GPIOx->group], pins, GPIO_Mode, GPIO_Type);
    LibGPIO_ConfGroup(&LIB_PORT_NS[GPIOx->group], pins, GPIO_Mode, GPIO_Type);
}

void LibGPIO_Write(const GPIO_TypeDef *GPIOx, uint16_t value)
{
    uint32_t mask = LibGPIO_Outputs[GPIOx->group] & (0xFFFFUL << GPIOx->shift);
    uint32_t bits = (uint32_t)value << GPIOx->shift;

    LIB_PORT_SEC[GPIOx->group].OUTSET = bits & mask;
    LIB_PORT_SEC[GPIOx->group].OUTCLR = ~bits & mask;
    LIB_PORT_NS[GPIOx->group].OUTSET = bits & mask;
    LIB_PORT_NS[GPIOx->group].OUTCLR = ~bits & mask;
}

static void LibGPIO_PeripheralGroup(Lib_PortGroup_t *port, uint32_t pin, uint32_t pmux)
{
    uint8_t v = port->PMUX[pin >> 1];

    if(pin & 1U)
        v = (uint8_t)((v & 0x0FU) | (pmux << 4));
    else
        v = (uint8_t)((v & 0xF0U) | pmux);
    port->PMUX[pin >> 1] = v;
    /* the input buffer stays off for the analog function */
    port->PINCFG[pin] = (uint8_t)((pmux == LIB_PMUX_B) ? LIB_PINCFG_PMUXEN : (LIB_PINCFG_PMUXEN | LIB_PINCFG_INEN));
}

void LibGPIO_InitPeripheral(uint32_t group, uint32_t pin, uint32_t pmux)
{
    if((group > 1U) || (pin > 31U))
        return;
    LibGPIO_Outputs[group] &= ~(1UL << pin);
    LibGPIO_PeripheralGroup(&LIB_PORT_SEC[group], pin, pmux);
    LibGPIO_PeripheralGroup(&LIB_PORT_NS[group], pin, pmux);
}
