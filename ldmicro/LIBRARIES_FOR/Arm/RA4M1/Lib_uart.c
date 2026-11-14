/**
 * LDmicro library for Renesas RA4M1 (Arduino Nano R4), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_uart.h"
#include "Lib_usr.h"

/* Bits of the serial status register (SSR) */
#define SSR_TDRE 0x80U
#define SSR_RDRF 0x40U
#define SSR_ORER 0x20U
#define SSR_FER 0x10U
#define SSR_PER 0x08U

void LibUart_Init(USART_TypeDef *USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops)
{
    uint32_t pclk = LIB_PCLKA_HZ;
    uint32_t n, mode;
    uint32_t bestErr = 0xFFFFFFFFU, bestBrr = 0, bestN = 0, bestMode = 0;

    (void)datalen;
    (void)parity;
    (void)stops;

    if((USARTx != R_SCI2) || (baudrate == 0))
        return;

    /* Module stop of SCI2: MSTPB29 */
    R_SYSTEM->PRCR = LIB_PRCR_UNLOCK;
    R_MSTP->MSTPCRB &= ~(1UL << 29);
    R_SYSTEM->PRCR = LIB_PRCR_LOCK;

    USARTx->SCR = 0; /* transmitter and receiver off, on-chip baud rate generator */

    /*
     * Bit rate = PCLKA / (divider * 4^n * (BRR + 1)), n = SMR.CKS, divider =
     *   32 with SEMR.BGDM = 0 and SEMR.ABCS = 0   (mode 0)
     *   16 with SEMR.BGDM = 1 and SEMR.ABCS = 0   (mode 1)
     *    8 with SEMR.BGDM = 1 and SEMR.ABCS = 1   (mode 2)
     * The combination with the smallest error is kept.
     */
    for(n = 0; n < 4; n++) {
        for(mode = 0; mode < 3; mode++) {
            uint32_t div = (32U >> mode) << (2U * n);
            uint32_t brr = (pclk + (div * baudrate) / 2U) / (div * baudrate);
            uint32_t real, err;

            if((brr == 0) || (brr > 256U))
                continue;
            real = pclk / (div * brr);
            err = (real > baudrate) ? (real - baudrate) : (baudrate - real);
            if(err < bestErr) {
                bestErr = err;
                bestBrr = brr - 1U;
                bestN = n;
                bestMode = mode;
            }
        }
    }
    if(bestErr == 0xFFFFFFFFU)
        return; /* baud rate out of range */

    USARTx->SMR = (uint8_t)bestN; /* asynchronous, 8 bits, no parity, 1 stop bit, clock PCLKA / 4^n */
    USARTx->SCMR = 0xF2;          /* not a smart card interface, LSB first, 8 bits */
    USARTx->SEMR = (uint8_t)(((bestMode >= 1U) ? 0x40U : 0U) | ((bestMode == 2U) ? 0x10U : 0U)); /* BGDM, ABCS */
    USARTx->BRR = (uint8_t)bestBrr;

    /* RXD2 = P301, TXD2 = P302 */
    LibGPIO_InitPeripheral(3, 1, LIB_PSEL_SCI_EVEN);
    LibGPIO_InitPeripheral(3, 2, LIB_PSEL_SCI_EVEN);

    delay_us((uint16_t)((1000000U / baudrate) + 1U)); /* at least one bit period */

    USARTx->SCR = 0x30; /* TE and RE together */
}

uint8_t LibUart_Received_Data(USART_TypeDef *USARTx)
{
    uint8_t ssr = USARTx->SSR;

    if(ssr & (SSR_ORER | SSR_FER | SSR_PER)) {
        /* a reception error blocks the receiver: read the data and clear the error flags */
        (void)USARTx->RDR;
        USARTx->SSR = (uint8_t)(0xC0U | (ssr & 0x07U)); /* ORER, FER, PER = 0; the other flags are not changed */
        return 0;
    }
    return (ssr & SSR_RDRF) ? 1 : 0;
}

uint8_t LibUart_Transmit_Ready(USART_TypeDef *USARTx)
{
    return (USARTx->SSR & SSR_TDRE) ? 1 : 0;
}

uint8_t LibUart_Getc(USART_TypeDef *USARTx)
{
    return USARTx->RDR; /* reading RDR clears RDRF */
}

void LibUart_Putc(USART_TypeDef *USARTx, volatile char c)
{
    uint32_t n;

    for(n = 0; ((USARTx->SSR & SSR_TDRE) == 0) && (n < 1000000U); n++)
        ;
    USARTx->TDR = (uint8_t)c; /* writing TDR clears TDRE */
}
