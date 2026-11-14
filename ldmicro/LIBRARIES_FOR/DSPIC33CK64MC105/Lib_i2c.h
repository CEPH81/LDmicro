/**
 * LDmicro library for Microchip dsPIC33CK64MC105 (dsPIC33CK64MC105 Curiosity Nano), for the
 * MPLAB XC16 / XC-DSC compiler.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_I2C_H
#define LIB_I2C_H

/*
    Pins used (master mode; external pull-up resistors are needed, there are none on the board):

    I2Cx    SCL             SDA
    I2C1    RC9 (ASCL1)     RC8 (ASDA1)     alternate pins of I2C1, chosen by a configuration bit
*/

#include "Lib_mcu.h"

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
