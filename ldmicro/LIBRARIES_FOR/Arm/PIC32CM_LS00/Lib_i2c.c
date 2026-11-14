/**
 * LDmicro library for Microchip PIC32CM LS00 (PIC32CM LS00 Curiosity Nano+ Touch, PIC32CM5164LS00048).
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_i2c.h"

#define LIB_I2C_TIMEOUT 200000U

/* INTFLAG */
#define I2C_MB 0x01U    /* master on bus: byte sent (or address not acknowledged, arbitration lost) */
#define I2C_SB 0x02U    /* slave on bus: byte received */
#define I2C_ERROR 0x80U
/* STATUS */
#define I2C_BUSERR 0x0001U
#define I2C_ARBLOST 0x0002U
#define I2C_RXNACK 0x0004U
#define I2C_BUSSTATE 0x0030U
#define I2C_BUS_IDLE 0x0010U
#define I2C_BUS_OWNER 0x0020U
/* CTRLB */
#define I2C_CMD_STOP (3UL << 16)
#define I2C_ACKACT (1UL << 18) /* 1 = answer NACK */

static void LibI2C_Sync(I2C_TypeDef *I2Cx)
{
    uint32_t n;

    for(n = 0; I2Cx->SYNCBUSY && (n < LIB_I2C_TIMEOUT); n++)
        ;
}

void LibI2C_MasterInit(I2C_TypeDef *I2Cx, uint32_t clockSpeed)
{
    uint32_t baud;

    if(I2Cx != I2C1)
        return;
    if(clockSpeed == 0)
        clockSpeed = 100000U;
    if(clockSpeed > 400000U)
        clockSpeed = 400000U;

    LibClock_Enable(LIB_GCLK_SERCOM1);

    I2Cx->CTRLA = LIB_SERCOM_SWRST;
    LibI2C_Sync(I2Cx);

    /*
     * SCL period = (10 + 2 * BAUD) clocks + rise time: the rise time is not known here,
     * the real rate is a little below the value asked for.
     */
    baud = SystemCoreClock / (2U * clockSpeed);
    baud = (baud > 5U) ? (baud - 5U) : 1U;
    if(baud > 255U)
        baud = 255U;

    I2Cx->CTRLA = (5UL << 2) | (2UL << 20); /* I2C master, SDA hold time 300-600 ns */
    I2Cx->CTRLB = 0;                        /* no smart mode: every step is a command of the library */
    LibI2C_Sync(I2Cx);
    I2Cx->BAUD32 = baud;

    /* PA16 = SERCOM1 PAD0 (SDA), PA17 = SERCOM1 PAD1 (SCL): function C */
    LibGPIO_InitPeripheral(0, 16, LIB_PMUX_C);
    LibGPIO_InitPeripheral(0, 17, LIB_PMUX_C);

    I2Cx->CTRLA |= LIB_SERCOM_ENABLE;
    LibI2C_Sync(I2Cx);

    /* the state of the bus is unknown after the start: force it to idle */
    I2Cx->STATUS = I2C_BUS_IDLE;
    LibI2C_Sync(I2Cx);
}

/* Waits for one of the flags of INTFLAG; returns 0 if it is set without bus error */
static int LibI2C_Wait(I2C_TypeDef *I2Cx, uint8_t flags)
{
    uint32_t n;

    for(n = 0; n < LIB_I2C_TIMEOUT; n++) {
        if(I2Cx->INTFLAG & flags)
            return (I2Cx->STATUS & (I2C_BUSERR | I2C_ARBLOST)) ? -1 : 0;
    }
    return -1;
}

/* Stop condition (when the master owns the bus) and flags cleared */
static void LibI2C_Stop(I2C_TypeDef *I2Cx)
{
    if((I2Cx->STATUS & I2C_BUSSTATE) == I2C_BUS_OWNER) {
        I2Cx->CTRLB = (I2Cx->CTRLB & ~I2C_ACKACT) | I2C_CMD_STOP;
        LibI2C_Sync(I2Cx);
    }
    I2Cx->INTFLAG = I2C_MB | I2C_SB | I2C_ERROR;
}

/* Ends a transfer after an error */
static void LibI2C_Abort(I2C_TypeDef *I2Cx)
{
    LibI2C_Stop(I2Cx);
    I2Cx->STATUS = I2C_BUSERR | I2C_ARBLOST; /* clear the error flags */
    if((I2Cx->STATUS & I2C_BUSSTATE) != I2C_BUS_IDLE) {
        I2Cx->STATUS = I2C_BUS_IDLE;
        LibI2C_Sync(I2Cx);
    }
    I2Cx->CTRLB &= ~I2C_ACKACT;
    LibI2C_Sync(I2Cx);
}

/* START + address (write) + register; returns 0 when the slave acknowledged both */
static int LibI2C_SendRegister(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    I2Cx->INTFLAG = I2C_MB | I2C_SB | I2C_ERROR;
    I2Cx->ADDR = (uint32_t)address << 1; /* start condition, slave address + W */
    LibI2C_Sync(I2Cx);
    if((LibI2C_Wait(I2Cx, I2C_MB) != 0) || (I2Cx->STATUS & I2C_RXNACK))
        return -1;

    I2Cx->DATA = reg;
    LibI2C_Sync(I2Cx);
    if((LibI2C_Wait(I2Cx, I2C_MB) != 0) || (I2Cx->STATUS & I2C_RXNACK))
        return -1;
    return 0;
}

void LibI2C_SetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg, uint8_t data)
{
    if((I2Cx->CTRLA & LIB_SERCOM_ENABLE) == 0)
        return; /* not initialised */

    if(LibI2C_SendRegister(I2Cx, address, reg) != 0)
        goto error;
    I2Cx->DATA = data;
    LibI2C_Sync(I2Cx);
    if((LibI2C_Wait(I2Cx, I2C_MB) != 0) || (I2Cx->STATUS & I2C_RXNACK))
        goto error;
    LibI2C_Stop(I2Cx);
    return;

error:
    LibI2C_Abort(I2Cx);
}

int16_t LibI2C_GetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    int16_t data;

    if((I2Cx->CTRLA & LIB_SERCOM_ENABLE) == 0)
        return -1; /* not initialised */

    if(LibI2C_SendRegister(I2Cx, address, reg) != 0)
        goto error;

    /* Repeated start, slave address + R: the first byte is received when the slave acknowledges */
    I2Cx->ADDR = ((uint32_t)address << 1) | 1U;
    LibI2C_Sync(I2Cx);
    if(LibI2C_Wait(I2Cx, I2C_SB | I2C_MB) != 0)
        goto error;
    if((I2Cx->INTFLAG & I2C_SB) == 0)
        goto error; /* address not acknowledged */

    /* Single byte: answer NACK and send the stop condition, then read the byte */
    I2Cx->CTRLB |= I2C_ACKACT;
    LibI2C_Sync(I2Cx);
    I2Cx->CTRLB = (I2Cx->CTRLB & ~(3UL << 16)) | I2C_CMD_STOP;
    LibI2C_Sync(I2Cx);
    data = (int16_t)(I2Cx->DATA & 0xFFU);
    I2Cx->CTRLB &= ~I2C_ACKACT;
    LibI2C_Sync(I2Cx);
    I2Cx->INTFLAG = I2C_MB | I2C_SB | I2C_ERROR;
    return data;

error:
    LibI2C_Abort(I2Cx);
    return -1;
}
