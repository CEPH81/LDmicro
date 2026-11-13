/**
 * LDmicro library for STM32L4 (STM32L432), written on the CMSIS device header.
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#include "Lib_uart.h"

void LibUart_Init(USART_TypeDef *USARTx, uint32_t baudrate, uint16_t datalen, uint16_t parity, uint16_t stops)
{
    volatile uint32_t tmp;

    (void)datalen;
    (void)parity;
    (void)stops;

    if(baudrate == 0)
        return;

    if(USARTx == USART1) {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
        tmp = RCC->APB2ENR;
        LibGPIO_InitAlternate(GPIOA, GPIO_Pin_9 | GPIO_Pin_10, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_50MHz, 7); /* AF7 */
    } else if(USARTx == USART2) {
        RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
        tmp = RCC->APB1ENR1;
        LibGPIO_InitAlternate(GPIOA, GPIO_Pin_2, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_50MHz, 7);  /* TX: AF7 */
        LibGPIO_InitAlternate(GPIOA, GPIO_Pin_15, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_50MHz, 3); /* RX: AF3 */
    } else
        return;
    (void)tmp;

    /* The kernel clock of the USART is PCLK = SystemCoreClock (reset value of RCC_CCIPR) */
    USARTx->CR1 = 0; /* disabled, 8 bits, no parity, oversampling by 16 */
    USARTx->CR2 = 0; /* 1 stop bit */
    USARTx->CR3 = 0; /* no flow control */
    USARTx->BRR = (SystemCoreClock + baudrate / 2U) / baudrate;
    USARTx->CR1 = USART_CR1_TE | USART_CR1_RE;
    USARTx->CR1 |= USART_CR1_UE;
}

uint8_t LibUart_Received_Data(USART_TypeDef *USARTx)
{
    if(USARTx->ISR & USART_ISR_ORE) /* an overrun would block the reception */
        USARTx->ICR = USART_ICR_ORECF;
    return (USARTx->ISR & USART_ISR_RXNE) ? 1 : 0;
}

uint8_t LibUart_Transmit_Ready(USART_TypeDef *USARTx)
{
    return (USARTx->ISR & USART_ISR_TXE) ? 1 : 0;
}

uint8_t LibUart_Getc(USART_TypeDef *USARTx)
{
    return (uint8_t)(USARTx->RDR & 0xFFU);
}

void LibUart_Putc(USART_TypeDef *USARTx, volatile char c)
{
    uint32_t n;

    for(n = 0; ((USARTx->ISR & USART_ISR_TXE) == 0) && (n < 1000000U); n++)
        ;
    USARTx->TDR = (uint8_t)c;
}
