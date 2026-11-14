/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_i2c.h"

#define LIB_I2C_TIMEOUT 60000U

/* I2C1CONL */
#define I2C_SEN 0x0001U
#define I2C_RSEN 0x0002U
#define I2C_PEN 0x0004U
#define I2C_RCEN 0x0008U
#define I2C_ACKEN 0x0010U
#define I2C_ACKDT 0x0020U
#define I2C_I2CEN 0x8000U
/* I2C1STAT */
#define I2C_TBF 0x0001U
#define I2C_RBF 0x0002U
#define I2C_BCL 0x0400U
#define I2C_TRSTAT 0x4000U
#define I2C_ACKSTAT 0x8000U

void LibI2C_MasterInit(I2C_TypeDef *I2Cx, uint32_t clockSpeed)
{
    uint32_t brg;

    if(I2Cx != I2C1)
        return;
    if(clockSpeed == 0)
        clockSpeed = 100000UL;
    if(clockSpeed > 400000UL)
        clockSpeed = 400000UL;

    I2C1CONL = 0; /* module off */

    /* SCL period = 2 * (BRG + 2) peripheral clocks, plus a fixed delay of the pins */
    brg = SystemCoreClock / (2UL * clockSpeed);
    brg = (brg > 2UL) ? (brg - 2UL) : 1UL;
    if(brg < 2UL)
        brg = 2UL; /* the values 0 and 1 are not allowed */
    I2C1BRG = (uint16_t)brg;

    I2C1STAT = 0;
    I2C1CONL = I2C_I2CEN;
}

/* Waits until the bits of I2C1CONL given by the mask are cleared by the module */
static int8_t LibI2C_WaitCon(uint16_t mask)
{
    uint16_t n;

    for(n = 0; (I2C1CONL & mask) && (n < LIB_I2C_TIMEOUT); n++)
        ;
    return (int8_t)((I2C1CONL & mask) ? -1 : 0);
}

/* Sends one byte; returns 0 when the slave acknowledged it */
static int8_t LibI2C_Write(uint8_t byte)
{
    uint16_t n;

    I2C1TRN = byte;
    for(n = 0; (I2C1STAT & (I2C_TBF | I2C_TRSTAT)) && (n < LIB_I2C_TIMEOUT); n++)
        ;
    if(I2C1STAT & (I2C_TBF | I2C_TRSTAT | I2C_BCL | I2C_ACKSTAT))
        return -1;
    return 0;
}

static void LibI2C_Stop(void)
{
    I2C1CONL |= I2C_PEN;
    (void)LibI2C_WaitCon(I2C_PEN);
}

/* Ends a transfer after an error */
static void LibI2C_Abort(void)
{
    if(I2C1STAT & I2C_BCL) {
        /* bus collision: restart the module */
        I2C1CONL = 0;
        I2C1STAT = 0;
        I2C1CONL = I2C_I2CEN;
        return;
    }
    LibI2C_Stop();
}

/* START + address (write) + register; returns 0 when the slave acknowledged both */
static int8_t LibI2C_SendRegister(uint8_t address, uint8_t reg)
{
    I2C1CONL |= I2C_SEN;
    if(LibI2C_WaitCon(I2C_SEN) != 0)
        return -1;
    if(LibI2C_Write((uint8_t)(address << 1)) != 0)
        return -1;
    return LibI2C_Write(reg);
}

void LibI2C_SetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg, uint8_t data)
{
    (void)I2Cx;
    if((I2C1CONL & I2C_I2CEN) == 0)
        return; /* not initialised */

    if((LibI2C_SendRegister(address, reg) != 0) || (LibI2C_Write(data) != 0)) {
        LibI2C_Abort();
        return;
    }
    LibI2C_Stop();
}

int16_t LibI2C_GetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    uint16_t n;
    int16_t  data;

    (void)I2Cx;
    if((I2C1CONL & I2C_I2CEN) == 0)
        return -1; /* not initialised */

    if(LibI2C_SendRegister(address, reg) != 0)
        goto error;

    /* Repeated start, slave address + R */
    I2C1CONL |= I2C_RSEN;
    if(LibI2C_WaitCon(I2C_RSEN) != 0)
        goto error;
    if(LibI2C_Write((uint8_t)((address << 1) | 1U)) != 0)
        goto error;

    /* Reception of one byte, answered with NACK */
    I2C1CONL |= I2C_RCEN;
    for(n = 0; ((I2C1STAT & I2C_RBF) == 0) && (n < LIB_I2C_TIMEOUT); n++)
        ;
    if((I2C1STAT & I2C_RBF) == 0)
        goto error;
    data = (int16_t)(I2C1RCV & 0x00FFU);
    I2C1CONL |= I2C_ACKDT;
    I2C1CONL |= I2C_ACKEN;
    (void)LibI2C_WaitCon(I2C_ACKEN);
    I2C1CONL &= (uint16_t)~I2C_ACKDT;

    LibI2C_Stop();
    return data;

error:
    LibI2C_Abort();
    return -1;
}
