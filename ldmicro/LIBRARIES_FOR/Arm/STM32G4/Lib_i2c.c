/**
 * LDmicro library for STM32G4 (STM32G431), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_i2c.h"

#define LIB_I2C_TIMEOUT 200000U

void LibI2C_MasterInit(I2C_TypeDef *I2Cx, uint32_t clockSpeed)
{
    volatile uint32_t tmp;
    uint32_t          presc, tick, scll, sclh, sdadel, scldel;

    if(I2Cx != I2C1)
        return;
    if(clockSpeed == 0)
        clockSpeed = 100000U;

    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
    tmp = RCC->APB1ENR1;
    (void)tmp;

    /* SCL = PA15, SDA = PB7: AF4, open drain */
    LibGPIO_InitAlternate(GPIOA, GPIO_Pin_15, GPIO_OType_OD, GPIO_PuPd_UP, GPIO_Speed_50MHz, 4);
    LibGPIO_InitAlternate(GPIOB, GPIO_Pin_7, GPIO_OType_OD, GPIO_PuPd_UP, GPIO_Speed_50MHz, 4);

    I2Cx->CR1 = 0; /* disabled: analog filter on, digital filter off */

    /* Timings: the kernel clock of the I2C is PCLK1 = SystemCoreClock (reset value of RCC_CCIPR).
       'tick' is the frequency of the prescaled clock that counts SCLL, SCLH, SDADEL and SCLDEL. */
    if(clockSpeed <= 100000U) {
        /* standard mode: tick about 4 MHz, tLOW = half period, tHIGH 0.5 us shorter */
        presc = (SystemCoreClock + 3999999U) / 4000000U;
        if(presc > 16U)
            presc = 16U;
        if(presc == 0)
            presc = 1;
        tick = SystemCoreClock / presc;
        scll = tick / (2U * clockSpeed);
        if(scll > 256U)
            scll = 256U;
        if(scll < 4U)
            scll = 4U;
        sclh = scll - tick / 2000000U;
        if((sclh < 2U) || (sclh > scll))
            sclh = 2U;
        sdadel = tick / 2000000U;           /* 0.5 us  */
        scldel = (tick * 5U) / 4000000U;    /* 1.25 us */
    } else {
        /* fast mode: tick about 8 MHz, tLOW = 1.25 us and tHIGH = 0.5 us at 400 kHz */
        if(clockSpeed > 400000U)
            clockSpeed = 400000U;
        presc = (SystemCoreClock + 7999999U) / 8000000U;
        if(presc > 16U)
            presc = 16U;
        if(presc == 0)
            presc = 1;
        tick = SystemCoreClock / presc;
        scll = (tick / clockSpeed) / 2U;
        if(scll < 4U)
            scll = 4U;
        if(scll > 256U)
            scll = 256U;
        sclh = (tick / clockSpeed) * 2U / 10U;
        if(sclh < 2U)
            sclh = 2U;
        sdadel = tick / 8000000U;           /* 0.125 us */
        scldel = tick / 2000000U;           /* 0.5 us   */
    }
    if(sdadel > 15U)
        sdadel = 15U;
    if(scldel < 1U)
        scldel = 1U;
    if(scldel > 16U)
        scldel = 16U;

    I2Cx->TIMINGR = ((presc - 1U) << I2C_TIMINGR_PRESC_Pos) | ((scldel - 1U) << I2C_TIMINGR_SCLDEL_Pos) | (sdadel << I2C_TIMINGR_SDADEL_Pos)
                    | ((sclh - 1U) << I2C_TIMINGR_SCLH_Pos) | ((scll - 1U) << I2C_TIMINGR_SCLL_Pos);

    I2Cx->CR1 = I2C_CR1_PE;
}

/* Waits for one of the flags; returns 0 if the flag is set, -1 on NACK or time-out */
static int LibI2C_Wait(I2C_TypeDef *I2Cx, uint32_t flag)
{
    uint32_t n;

    for(n = 0; n < LIB_I2C_TIMEOUT; n++) {
        uint32_t isr = I2Cx->ISR;
        if(isr & flag)
            return 0;
        if(isr & I2C_ISR_NACKF)
            break;
    }
    return -1;
}

/* Ends a transfer after an error: clears the flags and releases the bus */
static void LibI2C_Abort(I2C_TypeDef *I2Cx)
{
    uint32_t n;

    if(I2Cx->ISR & I2C_ISR_BUSY) {
        I2Cx->CR2 |= I2C_CR2_STOP;
        for(n = 0; ((I2Cx->ISR & I2C_ISR_STOPF) == 0) && (n < LIB_I2C_TIMEOUT); n++)
            ;
    }
    I2Cx->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF | I2C_ICR_BERRCF | I2C_ICR_ARLOCF;
    if(I2Cx->ISR & I2C_ISR_BUSY) {
        /* bus still held: restart the peripheral */
        I2Cx->CR1 &= ~I2C_CR1_PE;
        for(n = 0; n < 16; n++)
            (void)I2Cx->CR1;
        I2Cx->CR1 |= I2C_CR1_PE;
    }
}

static void LibI2C_Start(I2C_TypeDef *I2Cx, uint8_t address, uint32_t nbytes, uint32_t options)
{
    uint32_t cr2 = I2Cx->CR2;

    cr2 &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP | I2C_CR2_ADD10);
    cr2 |= (((uint32_t)address << 1) & I2C_CR2_SADD) | (nbytes << I2C_CR2_NBYTES_Pos) | options | I2C_CR2_START;
    I2Cx->CR2 = cr2;
}

void LibI2C_SetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg, uint8_t data)
{
    if((I2Cx->CR1 & I2C_CR1_PE) == 0)
        return;
    I2Cx->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;

    /* START + address (write) + register + data + STOP */
    LibI2C_Start(I2Cx, address, 2, I2C_CR2_AUTOEND);
    if(LibI2C_Wait(I2Cx, I2C_ISR_TXIS) != 0)
        goto error;
    I2Cx->TXDR = reg;
    if(LibI2C_Wait(I2Cx, I2C_ISR_TXIS) != 0)
        goto error;
    I2Cx->TXDR = data;
    if(LibI2C_Wait(I2Cx, I2C_ISR_STOPF) != 0)
        goto error;
    I2Cx->ICR = I2C_ICR_STOPCF;
    return;

error:
    LibI2C_Abort(I2Cx);
}

int16_t LibI2C_GetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    int16_t data;

    if((I2Cx->CR1 & I2C_CR1_PE) == 0)
        return -1;
    I2Cx->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;

    /* START + address (write) + register */
    LibI2C_Start(I2Cx, address, 1, 0);
    if(LibI2C_Wait(I2Cx, I2C_ISR_TXIS) != 0)
        goto error;
    I2Cx->TXDR = reg;
    if(LibI2C_Wait(I2Cx, I2C_ISR_TC) != 0)
        goto error;

    /* repeated START + address (read) + 1 byte + NACK + STOP */
    LibI2C_Start(I2Cx, address, 1, I2C_CR2_RD_WRN | I2C_CR2_AUTOEND);
    if(LibI2C_Wait(I2Cx, I2C_ISR_RXNE) != 0)
        goto error;
    data = (int16_t)(I2Cx->RXDR & 0xFFU);
    if(LibI2C_Wait(I2Cx, I2C_ISR_STOPF) != 0)
        goto error;
    I2Cx->ICR = I2C_ICR_STOPCF;
    return data;

error:
    LibI2C_Abort(I2Cx);
    return -1;
}
