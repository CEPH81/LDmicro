/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_gpio.h"

/* Output pins configured by the PLC program, for PORTA to PORTF */
static uint8_t LibGPIO_Outputs[6];

static uint8_t LibGPIO_Index(GPIO_TypeDef *GPIOx)
{
    uint8_t i = (uint8_t)(GPIOx - &PORTA);

    return (uint8_t)((i < 6U) ? i : 0U);
}

void LibGPIO_Conf(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t GPIO_Mode, uint8_t GPIO_Type, uint8_t GPIO_Speed)
{
    uint8_t           pins = (uint8_t)GPIO_Pin, pin;
    volatile uint8_t *pinctrl = &GPIOx->PIN0CTRL;

    (void)GPIO_Speed;

    if(GPIO_Mode == GPIO_Mode_OUT) {
        LibGPIO_Outputs[LibGPIO_Index(GPIOx)] |= pins;
        GPIOx->DIRSET = pins;
        return;
    }

    LibGPIO_Outputs[LibGPIO_Index(GPIOx)] &= (uint8_t)~pins;
    GPIOx->DIRCLR = pins;
    for(pin = 0; pin < 8; pin++) {
        if(pins & (uint8_t)(1U << pin)) {
            if(GPIO_Type == GPIO_PuPd_UP)
                pinctrl[pin] |= PORT_PULLUPEN_bm;
            else
                pinctrl[pin] &= (uint8_t)~PORT_PULLUPEN_bm;
        }
    }
}

void LibGPIO_Write(GPIO_TypeDef *GPIOx, uint8_t value)
{
    uint8_t mask = LibGPIO_Outputs[LibGPIO_Index(GPIOx)];

    GPIOx->OUTSET = (uint8_t)(value & mask);
    GPIOx->OUTCLR = (uint8_t)(~value & mask);
}
