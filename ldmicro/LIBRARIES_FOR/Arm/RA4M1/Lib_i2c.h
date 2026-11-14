/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_I2C_H
#define LIB_I2C_H

/*
    Pins used (master mode; external pull-up resistors are needed):

    I2Cx    SCL             SDA
    I2C1    P100 (A5)       P101 (A4)       IIC1
*/

#include "Lib_mcu.h"
#include "Lib_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* clockSpeed in Hz, up to 400000 */
void LibI2C_MasterInit(I2C_TypeDef *I2Cx, uint32_t clockSpeed);

/* Reads one register of a slave; address = 7 bits address; returns -1 on error */
int16_t LibI2C_GetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg);

/* Writes one register of a slave; address = 7 bits address */
void LibI2C_SetReg(I2C_TypeDef *I2Cx, uint8_t address, uint8_t reg, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif
