/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_i2c.h"

#define LIB_I2C_TIMEOUT 200000U

/* ICCR2 */
#define ICCR2_ST 0x02U
#define ICCR2_RS 0x04U
#define ICCR2_SP 0x08U
#define ICCR2_BBSY 0x80U
/* ICSR2 */
#define ICSR2_START 0x04U
#define ICSR2_STOP 0x08U
#define ICSR2_NACKF 0x10U
#define ICSR2_RDRF 0x20U
#define ICSR2_TEND 0x40U
#define ICSR2_TDRE 0x80U
/* ICMR3 */
#define ICMR3_ACKBT 0x08U
#define ICMR3_ACKWP 0x10U
#define ICMR3_WAIT 0x40U

void LibI2C_MasterInit(I2C_TypeDef *I2Cx, uint32_t clockSpeed)
{
    uint32_t cks, ref, low, high;

    if(I2Cx != R_IIC1)
        return;
    if(clockSpeed == 0)
        clockSpeed = 100000U;
    if(clockSpeed > 400000U)
        clockSpeed = 400000U;

    /* Module stop of IIC1: MSTPB8 */
    R_SYSTEM->PRCR = LIB_PRCR_UNLOCK;
    R_MSTP->MSTPCRB &= ~(1UL << 8);
    R_SYSTEM->PRCR = LIB_PRCR_LOCK;

    I2Cx->ICCR1 = 0;    /* ICE = 0: SCL and SDA not driven */
    I2Cx->ICCR1 = 0x40; /* IICRST = 1: reset of the module... */
    I2Cx->ICCR1 = 0xC0; /* ...then internal reset with ICE = 1 */

    /*
     * Bit rate: the low and high widths of SCL are (ICBRL + 1) and (ICBRH + 1) periods of the
     * internal reference clock PCLKB / 2^CKS, to which the hardware adds the rise time and the
     * delay of the noise filter: the real rate is a little below the value computed here.
     *   up to 100 kHz: low = high = half a period
     *   above        : low = 60 %, high = 40 % of the period (fast mode)
     */
    for(cks = 0; cks < 8; cks++) {
        ref = LIB_PCLKB_HZ >> cks;
        if(clockSpeed <= 100000U) {
            low = (ref + 2U * clockSpeed - 1U) / (2U * clockSpeed);
            high = low;
        } else {
            low = (ref * 6U + 10U * clockSpeed - 1U) / (10U * clockSpeed);
            high = (ref * 4U + 10U * clockSpeed - 1U) / (10U * clockSpeed);
        }
        if((low <= 32U) && (high <= 32U))
            break;
    }
    if(cks > 7U)
        cks = 7U;
    if(low < 2U)
        low = 2U;
    if(low > 32U)
        low = 32U;
    if(high < 2U)
        high = 2U;
    if(high > 32U)
        high = 32U;

    I2Cx->ICSER = 0;                               /* no slave address */
    I2Cx->ICMR1 = (uint8_t)(0x08U | (cks << 4));   /* BCWP = 1 (bit counter not written), CKS */
    I2Cx->ICBRL = (uint8_t)(0xE0U | (low - 1U));   /* the 3 upper bits are written as 1 */
    I2Cx->ICBRH = (uint8_t)(0xE0U | (high - 1U));
    I2Cx->ICMR2 = 0x06;                            /* reset value */
    I2Cx->ICMR3 = 0;
    I2Cx->ICFER = 0x72;                            /* arbitration lost detection, NACK suspension, noise filter, SCL synchronisation */
    I2Cx->ICIER = 0;                               /* no interrupt: the flags are polled */

    /* SCL1 = P100, SDA1 = P101 */
    LibGPIO_InitPeripheral(1, 0, LIB_PSEL_IIC);
    LibGPIO_InitPeripheral(1, 1, LIB_PSEL_IIC);

    I2Cx->ICCR1 = 0x9F; /* ICE = 1, internal reset released, SCL and SDA released */
}

/* Waits for a flag of ICSR2; returns 0 if the flag is set, -1 on NACK (when checked) or time-out */
static int LibI2C_Wait(I2C_TypeDef *I2Cx, uint8_t flag, int checkNack)
{
    uint32_t n;

    for(n = 0; n < LIB_I2C_TIMEOUT; n++) {
        uint8_t sr = I2Cx->ICSR2;
        if(checkNack && (sr & ICSR2_NACKF))
            return -1;
        if(sr & flag)
            return 0;
    }
    return -1;
}

static int LibI2C_BusFree(I2C_TypeDef *I2Cx)
{
    uint32_t n;

    for(n = 0; (I2Cx->ICCR2 & ICCR2_BBSY) && (n < LIB_I2C_TIMEOUT); n++)
        ;
    return (I2Cx->ICCR2 & ICCR2_BBSY) ? -1 : 0;
}

/* Issues a stop condition and clears the flags */
static void LibI2C_Stop(I2C_TypeDef *I2Cx)
{
    I2Cx->ICSR2 &= (uint8_t)~ICSR2_STOP;
    I2Cx->ICCR2 = (uint8_t)((I2Cx->ICCR2 & 0xE0U) | ICCR2_SP); /* the ST, RS, SP bits are written as 0 except SP */
    (void)LibI2C_Wait(I2Cx, ICSR2_STOP, 0);
    I2Cx->ICSR2 &= (uint8_t)~(ICSR2_NACKF | ICSR2_STOP);
}

/* Ends a transfer after an error */
static void LibI2C_Abort(I2C_TypeDef *I2Cx)
{
    if(I2Cx->ICCR2 & ICCR2_BBSY)
        LibI2C_Stop(I2Cx);
    if(I2Cx->ICCR2 & ICCR2_BBSY) {
        /* bus still held: internal reset of the module, settings kept */
        I2Cx->ICCR1 |= 0x40;
        I2Cx->ICCR1 &= (uint8_t)~0x40U;
    }
    I2Cx->ICMR3 &= (uint8_t)~ICMR3_WAIT;
    I2Cx->ICSR2 = 0;
}

/* START + address (write) + register; returns 0 when the register byte has been sent */
static int LibI2C_SendRegister(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    if(LibI2C_BusFree(I2Cx) != 0)
        return -1;
    I2Cx->ICSR2 = 0;
    I2Cx->ICCR2 = (uint8_t)((I2Cx->ICCR2 & 0xE0U) | ICCR2_ST); /* start condition */

    if(LibI2C_Wait(I2Cx, ICSR2_TDRE, 1) != 0)
        return -1;
    I2Cx->ICDRT = (uint8_t)(address << 1); /* slave address + W */
    if(LibI2C_Wait(I2Cx, ICSR2_TDRE, 1) != 0)
        return -1;
    I2Cx->ICDRT = reg;
    return 0;
}

void LibI2C_SetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg, uint8_t data)
{
    if((I2Cx->ICCR1 & 0x80U) == 0)
        return; /* not initialised */

    if(LibI2C_SendRegister(I2Cx, address, reg) != 0)
        goto error;
    if(LibI2C_Wait(I2Cx, ICSR2_TDRE, 1) != 0)
        goto error;
    I2Cx->ICDRT = data;
    if(LibI2C_Wait(I2Cx, ICSR2_TEND, 1) != 0)
        goto error;
    LibI2C_Stop(I2Cx);
    return;

error:
    LibI2C_Abort(I2Cx);
}

int16_t LibI2C_GetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    int16_t data;

    if((I2Cx->ICCR1 & 0x80U) == 0)
        return -1; /* not initialised */

    if(LibI2C_SendRegister(I2Cx, address, reg) != 0)
        goto error;
    if(LibI2C_Wait(I2Cx, ICSR2_TEND, 1) != 0)
        goto error;

    /* Restart condition, then slave address + R */
    I2Cx->ICSR2 &= (uint8_t)~ICSR2_START;
    I2Cx->ICCR2 = (uint8_t)((I2Cx->ICCR2 & 0xE0U) | ICCR2_RS);
    if(LibI2C_Wait(I2Cx, ICSR2_START, 0) != 0)
        goto error;
    if(LibI2C_Wait(I2Cx, ICSR2_TDRE, 0) != 0)
        goto error;
    I2Cx->ICDRT = (uint8_t)((address << 1) | 1U);

    /* RDRF is set when the address has been acknowledged */
    if(LibI2C_Wait(I2Cx, ICSR2_RDRF, 1) != 0)
        goto error;

    /* Reception of a single byte: hold SCL low after the byte (WAIT), answer NACK, then the
       dummy read of ICDRR starts the reception */
    I2Cx->ICMR3 |= ICMR3_WAIT;
    I2Cx->ICMR3 |= ICMR3_ACKWP;
    I2Cx->ICMR3 = (uint8_t)(I2Cx->ICMR3 | ICMR3_ACKBT);
    I2Cx->ICMR3 &= (uint8_t)~ICMR3_ACKWP;
    (void)I2Cx->ICDRR;

    if(LibI2C_Wait(I2Cx, ICSR2_RDRF, 0) != 0)
        goto error;

    /* Stop condition requested before the last read, which releases SCL */
    I2Cx->ICSR2 &= (uint8_t)~ICSR2_STOP;
    I2Cx->ICCR2 = (uint8_t)((I2Cx->ICCR2 & 0xE0U) | ICCR2_SP);
    data = (int16_t)I2Cx->ICDRR;
    I2Cx->ICMR3 &= (uint8_t)~ICMR3_WAIT;
    (void)LibI2C_Wait(I2Cx, ICSR2_STOP, 0);
    I2Cx->ICSR2 &= (uint8_t)~(ICSR2_NACKF | ICSR2_STOP);
    return data;

error:
    LibI2C_Abort(I2Cx);
    return -1;
}
