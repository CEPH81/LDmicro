/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_gpio.h"

#ifdef ANSELD
#define LIB_ANSELD (&ANSELD)
#else
#define LIB_ANSELD ((volatile uint16_t *)0)
#endif

const GPIO_TypeDef LibGPIO_Ports[4] = {
    {&TRISA, &PORTA, &LATA, &ANSELA, &CNPUA, &CNPDA},
    {&TRISB, &PORTB, &LATB, &ANSELB, &CNPUB, &CNPDB},
    {&TRISC, &PORTC, &LATC, &ANSELC, &CNPUC, &CNPDC},
    {&TRISD, &PORTD, &LATD, LIB_ANSELD, &CNPUD, &CNPDD},
};

/* Output pins configured by the PLC program */
static uint16_t LibGPIO_Outputs[4];

void LibGPIO_Conf(const GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t GPIO_Mode, uint8_t GPIO_Type, uint8_t GPIO_Speed)
{
    uint8_t i = (uint8_t)(GPIOx - LibGPIO_Ports);

    (void)GPIO_Speed;
    if(i > 3U)
        return;

    if(GPIOx->ansel)
        *GPIOx->ansel &= (uint16_t)~GPIO_Pin; /* digital */

    if(GPIO_Mode == GPIO_Mode_OUT) {
        LibGPIO_Outputs[i] |= GPIO_Pin;
        *GPIOx->cnpu &= (uint16_t)~GPIO_Pin;
        *GPIOx->cnpd &= (uint16_t)~GPIO_Pin;
        *GPIOx->tris &= (uint16_t)~GPIO_Pin;
        return;
    }

    LibGPIO_Outputs[i] &= (uint16_t)~GPIO_Pin;
    *GPIOx->tris |= GPIO_Pin;
    if(GPIO_Type == GPIO_PuPd_UP) {
        *GPIOx->cnpd &= (uint16_t)~GPIO_Pin;
        *GPIOx->cnpu |= GPIO_Pin;
    } else if(GPIO_Type == GPIO_PuPd_DOWN) {
        *GPIOx->cnpu &= (uint16_t)~GPIO_Pin;
        *GPIOx->cnpd |= GPIO_Pin;
    } else {
        *GPIOx->cnpu &= (uint16_t)~GPIO_Pin;
        *GPIOx->cnpd &= (uint16_t)~GPIO_Pin;
    }
}

void LibGPIO_Write(const GPIO_TypeDef *GPIOx, uint16_t value)
{
    uint8_t  i = (uint8_t)(GPIOx - LibGPIO_Ports);
    uint16_t mask;

    if(i > 3U)
        return;
    mask = LibGPIO_Outputs[i];
    *GPIOx->lat = (uint16_t)((*GPIOx->lat & (uint16_t)~mask) | (value & mask));
}
