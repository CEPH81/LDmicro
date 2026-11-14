/**
 * LDmicro library for Renesas RA4M1 (R7FA4M1AB, Arduino Nano R4).
 *
 * This library is written directly on the CMSIS device header of Renesas
 * (R7FA4M1AB.h). This header provides the names that the C code generated
 * by LDmicro relies on (they are those of the STM32 libraries of LDmicro):
 *
 *   GPIOA..GPIOF    ports 0 to 5 of the RA4M1 (P0xx = GPIOA, P1xx = GPIOB...)
 *   TIM0..TIM7      channels 0 to 7 of the general PWM timer (GPT)
 *   TIM6            GPT6 is the timer of the PLC cycle
 *   USART2          SCI2 (D0 = RXD2, D1 = TXD2)
 *   ADC0, ADC1      the 14 bits A/D converter (ADC1 is used by the code
 *                   generator for the channels 16 and above)
 *   SPI1            SPI1 (D11 = MOSI, D12 = MISO, D13 = SCK)
 *   I2C1            IIC1 (A4 = SDA, A5 = SCL)
 *
 * Clocks set by SystemInit(): HOCO 48 MHz -> ICLK = PCLKA = PCLKC = PCLKD =
 * 48 MHz, PCLKB = FCLK = 24 MHz (same settings as the Arduino core).
 *
 * Distributed under the same license as LDmicro (GPL v3 or later).
 */
#ifndef LIB_MCU_H
#define LIB_MCU_H

#include <stdint.h>

/* The RA4M1 has 32 interrupt lines; each one is linked to a peripheral event with ICU->IELSRn.
   Line 0 is used for the PLC cycle timer (overflow of GPT6). */
typedef enum IRQn {
    Reset_IRQn = -15,
    NonMaskableInt_IRQn = -14,
    HardFault_IRQn = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn = -11,
    UsageFault_IRQn = -10,
    SVCall_IRQn = -5,
    DebugMonitor_IRQn = -4,
    PendSV_IRQn = -2,
    SysTick_IRQn = -1,
    TIM6_IRQn = 0,
    ICU_IRQ31_IRQn = 31
} IRQn_Type;

#include "R7FA4M1AB.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { RESET = 0, SET = !RESET } FlagStatus, ITStatus;
typedef enum { DISABLE = 0, ENABLE = !DISABLE } FunctionalState;

/* Clock frequencies after SystemInit() */
#define LIB_PCLKA_HZ (SystemCoreClock)      /* SCI, SPI */
#define LIB_PCLKB_HZ (SystemCoreClock / 2U) /* IIC */
#define LIB_PCLKD_HZ (SystemCoreClock)      /* GPT */

/* Protection of the clock and module stop registers */
#define LIB_PRCR_UNLOCK 0xA503U
#define LIB_PRCR_LOCK 0xA500U

/* Event number of the GPT6 overflow, linked to the interrupt line 0 */
#define LIB_EVENT_GPT6_OVERFLOW 141U
#define LIB_IELSR_IR (1UL << 16)

/* ------------------------------------------------------------------ GPIO */
typedef R_PORT0_Type GPIO_TypeDef;

#define GPIOA R_PORT0
#define GPIOB R_PORT1
#define GPIOC R_PORT2
#define GPIOD R_PORT3
#define GPIOE R_PORT4
#define GPIOF R_PORT5

#define GPIO_Pin_0 ((uint16_t)0x0001)
#define GPIO_Pin_1 ((uint16_t)0x0002)
#define GPIO_Pin_2 ((uint16_t)0x0004)
#define GPIO_Pin_3 ((uint16_t)0x0008)
#define GPIO_Pin_4 ((uint16_t)0x0010)
#define GPIO_Pin_5 ((uint16_t)0x0020)
#define GPIO_Pin_6 ((uint16_t)0x0040)
#define GPIO_Pin_7 ((uint16_t)0x0080)
#define GPIO_Pin_8 ((uint16_t)0x0100)
#define GPIO_Pin_9 ((uint16_t)0x0200)
#define GPIO_Pin_10 ((uint16_t)0x0400)
#define GPIO_Pin_11 ((uint16_t)0x0800)
#define GPIO_Pin_12 ((uint16_t)0x1000)
#define GPIO_Pin_13 ((uint16_t)0x2000)
#define GPIO_Pin_14 ((uint16_t)0x4000)
#define GPIO_Pin_15 ((uint16_t)0x8000)

#define GPIO_PIN_0 GPIO_Pin_0
#define GPIO_PIN_1 GPIO_Pin_1
#define GPIO_PIN_2 GPIO_Pin_2
#define GPIO_PIN_3 GPIO_Pin_3
#define GPIO_PIN_4 GPIO_Pin_4
#define GPIO_PIN_5 GPIO_Pin_5
#define GPIO_PIN_6 GPIO_Pin_6
#define GPIO_PIN_7 GPIO_Pin_7
#define GPIO_PIN_8 GPIO_Pin_8
#define GPIO_PIN_9 GPIO_Pin_9
#define GPIO_PIN_10 GPIO_Pin_10
#define GPIO_PIN_11 GPIO_Pin_11
#define GPIO_PIN_12 GPIO_Pin_12
#define GPIO_PIN_13 GPIO_Pin_13
#define GPIO_PIN_14 GPIO_Pin_14
#define GPIO_PIN_15 GPIO_Pin_15

#define GPIO_Mode_IN 0U
#define GPIO_Mode_OUT 1U

#define GPIO_OType_PP 0U
#define GPIO_OType_OD 1U

#define GPIO_PuPd_NOPULL 0U
#define GPIO_PuPd_UP 1U
#define GPIO_PuPd_DOWN 2U /* not available on the RA4M1: no pull-down resistors */

#define GPIO_Speed_2MHz 0U
#define GPIO_Speed_25MHz 1U
#define GPIO_Speed_50MHz 2U
#define GPIO_Speed_100MHz 3U

#define GPIO_ReadInputDataBit(GPIOx, pin) ((uint8_t)((((GPIOx)->PIDR) & (pin)) ? 1 : 0))
#define GPIO_SetBits(GPIOx, pin) ((GPIOx)->POSR = (uint16_t)(pin))
#define GPIO_ResetBits(GPIOx, pin) ((GPIOx)->PORR = (uint16_t)(pin))
/* Only the pins configured as outputs with LibGPIO_Conf() are written */
#define GPIO_Write(GPIOx, value) LibGPIO_Write((GPIOx), (uint16_t)(value))

/* ---------------------------------------------------------------- Timers */
/* A timer is the number of a GPT channel */
#define TIM0 0U
#define TIM1 1U
#define TIM2 2U
#define TIM3 3U
#define TIM4 4U
#define TIM5 5U
#define TIM6 6U
#define TIM7 7U

#define TIM_IT_Update 1U

#define TIM_GetITStatus(TIMx, it) ((R_ICU->IELSR[TIM6_IRQn] & LIB_IELSR_IR) ? SET : RESET)
#define TIM_ClearITPendingBit(TIMx, it) LibTimer_ClearInterrupt(TIMx)

/* ----------------------------------------------------------------- USART */
typedef R_SCI0_Type USART_TypeDef;

#define USART2 R_SCI2

#define USART_WordLength_8b 0U
#define USART_Parity_No 0U
#define USART_StopBits_1 0U

/* ------------------------------------------------------------------- ADC */
typedef R_ADC0_Type ADC_TypeDef;

#define ADC0 R_ADC0
#define ADC1 R_ADC0 /* same converter: the code generator derives this name from the channels 16 and above */

#define ADC_Resolution_12b 0U

#define ADC_Channel_0 0
#define ADC_Channel_1 1
#define ADC_Channel_2 2
#define ADC_Channel_3 3
#define ADC_Channel_4 4
#define ADC_Channel_5 5
#define ADC_Channel_6 6
#define ADC_Channel_7 7
#define ADC_Channel_8 8
#define ADC_Channel_9 9
#define ADC_Channel_10 10
#define ADC_Channel_11 11
#define ADC_Channel_12 12
#define ADC_Channel_13 13
#define ADC_Channel_14 14
#define ADC_Channel_16 16
#define ADC_Channel_17 17
#define ADC_Channel_18 18
#define ADC_Channel_19 19
#define ADC_Channel_20 20
#define ADC_Channel_21 21
#define ADC_Channel_22 22
#define ADC_Channel_23 23
#define ADC_Channel_24 24
#define ADC_Channel_25 25

/* ------------------------------------------------------------- SPI, I2C */
typedef R_SPI0_Type SPI_TypeDef;
typedef R_IIC0_Type I2C_TypeDef;

#define SPI1 R_SPI1
#define I2C1 R_IIC1

/* ------------------------------------------------------- Pin functions */
/* Values of the PSEL field of the PmnPFS registers */
#define LIB_PSEL_GPT 0x03U
#define LIB_PSEL_SCI_EVEN 0x04U /* SCI0, SCI2 */
#define LIB_PSEL_SCI_ODD 0x05U  /* SCI1, SCI9 */
#define LIB_PSEL_SPI 0x06U
#define LIB_PSEL_IIC 0x07U

void LibGPIO_Write(GPIO_TypeDef *GPIOx, uint16_t value);
void LibTimer_ClearInterrupt(uint32_t TIMx);

#ifdef __cplusplus
}
#endif

#endif
