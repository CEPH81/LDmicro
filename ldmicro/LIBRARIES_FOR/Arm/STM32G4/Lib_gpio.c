/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_gpio.h"

void LibGPIO_EnableClock(GPIO_TypeDef *GPIOx)
{
    volatile uint32_t tmp;

    if(GPIOx == GPIOA)
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    else if(GPIOx == GPIOB)
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    else if(GPIOx == GPIOC)
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    else if(GPIOx == GPIOD)
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
    else if(GPIOx == GPIOE)
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
    else if(GPIOx == GPIOF)
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOFEN;
    else if(GPIOx == GPIOG)
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;
    tmp = RCC->AHB2ENR; /* short delay after enabling the clock */
    (void)tmp;
}

void LibGPIO_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_OType, uint32_t GPIO_PuPd, uint32_t GPIO_Speed)
{
    uint32_t pin;

    if(GPIO_Pin == 0)
        return;

    LibGPIO_EnableClock(GPIOx);

    for(pin = 0; pin < 16; pin++) {
        if((GPIO_Pin & (1U << pin)) == 0)
            continue;

        if((GPIO_Mode == GPIO_Mode_OUT) || (GPIO_Mode == GPIO_Mode_AF)) {
            GPIOx->OSPEEDR = (GPIOx->OSPEEDR & ~(3U << (2 * pin))) | ((GPIO_Speed & 3U) << (2 * pin));
            GPIOx->OTYPER = (GPIOx->OTYPER & ~(1U << pin)) | ((GPIO_OType & 1U) << pin);
        }
        GPIOx->PUPDR = (GPIOx->PUPDR & ~(3U << (2 * pin))) | ((GPIO_PuPd & 3U) << (2 * pin));
        GPIOx->MODER = (GPIOx->MODER & ~(3U << (2 * pin))) | ((GPIO_Mode & 3U) << (2 * pin));
    }
}

void LibGPIO_Conf(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_Mode, uint32_t GPIO_Type, uint32_t GPIO_Speed)
{
    if(GPIO_Mode == GPIO_Mode_IN)
        LibGPIO_Init(GPIOx, GPIO_Pin, GPIO_Mode_IN, GPIO_OType_PP, GPIO_Type, GPIO_Speed); /* with or without pull-up / down */
    else
        LibGPIO_Init(GPIOx, GPIO_Pin, GPIO_Mode, GPIO_Type, GPIO_PuPd_NOPULL, GPIO_Speed); /* push-pull or open drain */
}

void LibGPIO_InitAlternate(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_OType, uint32_t GPIO_PuPd, uint32_t GPIO_Speed, uint8_t Alternate)
{
    uint32_t pin;

    if(GPIO_Pin == 0)
        return;

    LibGPIO_EnableClock(GPIOx);

    /* select the alternate function before switching the pin mode */
    for(pin = 0; pin < 16; pin++) {
        if((GPIO_Pin & (1U << pin)) == 0)
            continue;
        GPIOx->AFR[pin >> 3] = (GPIOx->AFR[pin >> 3] & ~(15U << (4 * (pin & 7U)))) | (((uint32_t)Alternate & 15U) << (4 * (pin & 7U)));
    }
    LibGPIO_Init(GPIOx, GPIO_Pin, GPIO_Mode_AF, GPIO_OType, GPIO_PuPd, GPIO_Speed);
}

void LibGPIO_InitAnalog(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    LibGPIO_Init(GPIOx, GPIO_Pin, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_2MHz);
}
