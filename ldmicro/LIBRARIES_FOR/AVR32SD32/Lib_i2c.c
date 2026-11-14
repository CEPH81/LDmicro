/**
 * LDmicro library for Microchip AVR32SD32 (AVR32SD32 Curiosity Nano), for avr-gcc.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_i2c.h"

#define LIB_I2C_TIMEOUT 60000U

void LibI2C_MasterInit(I2C_TypeDef *I2Cx, uint32_t clockSpeed)
{
    uint32_t baud;

    if(I2Cx != I2C0)
        return;
    if(clockSpeed == 0)
        clockSpeed = 100000UL;
    if(clockSpeed > 400000UL)
        clockSpeed = 400000UL;

    /*
     * SCL frequency = clock / (10 + 2 * MBAUD + clock * rise time): the rise time is not
     * known here, the real rate is a little below the value asked for.
     */
    baud = SystemCoreClock / (2UL * clockSpeed);
    baud = (baud > 5UL) ? (baud - 5UL) : 1UL;
    if(baud > 255UL)
        baud = 255UL;

    I2Cx->MCTRLA = 0;

    /* default pins of TWI0: PA2 = SDA, PA3 = SCL */
    PORTMUX.TWIROUTEA = (uint8_t)((PORTMUX.TWIROUTEA & (uint8_t)~PORTMUX_TWI0_gm) | PORTMUX_TWI0_DEFAULT_gc);

    I2Cx->MBAUD = (uint8_t)baud;
    I2Cx->MCTRLA = TWI_ENABLE_bm;
    I2Cx->MCTRLB = TWI_FLUSH_bm;
    I2Cx->MSTATUS = TWI_BUSSTATE_IDLE_gc; /* the state of the bus is unknown after the start: force it to idle */
}

/* Waits for the write (WIF) or read (RIF) flag; returns 0 when set without bus error */
static int8_t LibI2C_Wait(I2C_TypeDef *I2Cx)
{
    uint16_t n;

    for(n = 0; n < LIB_I2C_TIMEOUT; n++) {
        uint8_t status = I2Cx->MSTATUS;
        if(status & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
            return -1;
        if(status & (TWI_WIF_bm | TWI_RIF_bm))
            return 0;
    }
    return -1;
}

/* Ends a transfer after an error */
static void LibI2C_Abort(I2C_TypeDef *I2Cx)
{
    if((I2Cx->MSTATUS & TWI_BUSSTATE_gm) == TWI_BUSSTATE_OWNER_gc)
        I2Cx->MCTRLB = TWI_MCMD_STOP_gc;
    /* clear the flags and bring the bus state machine back to idle */
    I2Cx->MSTATUS = TWI_RIF_bm | TWI_WIF_bm | TWI_ARBLOST_bm | TWI_BUSERR_bm | TWI_BUSSTATE_IDLE_gc;
}

/* START + address (write) + register; returns 0 when the slave acknowledged both */
static int8_t LibI2C_SendRegister(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    I2Cx->MADDR = (uint8_t)(address << 1); /* start condition, slave address + W */
    if((LibI2C_Wait(I2Cx) != 0) || (I2Cx->MSTATUS & TWI_RXACK_bm))
        return -1;

    I2Cx->MDATA = reg;
    if((LibI2C_Wait(I2Cx) != 0) || (I2Cx->MSTATUS & TWI_RXACK_bm))
        return -1;
    return 0;
}

void LibI2C_SetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg, uint8_t data)
{
    if((I2Cx->MCTRLA & TWI_ENABLE_bm) == 0)
        return; /* not initialised */

    if(LibI2C_SendRegister(I2Cx, address, reg) != 0)
        goto error;
    I2Cx->MDATA = data;
    if((LibI2C_Wait(I2Cx) != 0) || (I2Cx->MSTATUS & TWI_RXACK_bm))
        goto error;
    I2Cx->MCTRLB = TWI_MCMD_STOP_gc;
    return;

error:
    LibI2C_Abort(I2Cx);
}

int16_t LibI2C_GetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg)
{
    int16_t data;

    if((I2Cx->MCTRLA & TWI_ENABLE_bm) == 0)
        return -1; /* not initialised */

    if(LibI2C_SendRegister(I2Cx, address, reg) != 0)
        goto error;

    /* Repeated start, slave address + R: the first byte is received when the slave acknowledges */
    I2Cx->MADDR = (uint8_t)((address << 1) | 1U);
    if(LibI2C_Wait(I2Cx) != 0)
        goto error;
    if((I2Cx->MSTATUS & TWI_RIF_bm) == 0)
        goto error; /* address not acknowledged */

    /* Single byte: answer NACK and send the stop condition, then read the byte */
    I2Cx->MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
    data = (int16_t)I2Cx->MDATA;
    return data;

error:
    LibI2C_Abort(I2Cx);
    return -1;
}
