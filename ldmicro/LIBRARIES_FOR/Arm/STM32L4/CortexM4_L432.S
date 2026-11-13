/************************************************************************************
 * File:    CortexM4_L432.S
 * Purpose: Startup file for the STM32L432 (Cortex-M4), for LDmicro.
 *          Same start-up code as CortexM4.S of the STM32F4 library;
 *          the interrupt names are those of the ST startup file with
 *          "_Handler" instead of "_IRQHandler" (TIM7_Handler...).
 *          Should use with GCC for ARM Embedded Processors
 * Version: V1.4
 * Date:    09 July 2012
 * Notice:  Changed for use with emIDE project wizard
 * Date:    05 July 2013
 *
 * Copyright (c) 2011, 2012, ARM Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the ARM Limited nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ARM LIMITED BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ************************************************************************************/

    .syntax unified
    .arch armv7-m

/************************************************************************************
 *
 *  The minimal vector table for Cortex-M4.
 *  Device specific external interrupts can be added below.
 ************************************************************************************/
    .section .isr_vector,"a",%progbits
    .align 2
    .globl __isr_vector
__isr_vector:
    .long    __stack_end__         /* Top of Stack */
    .long    Reset_Handler         /* Reset Handler */
    .long    NMI_Handler           /* NMI Handler */
    .long    HardFault_Handler     /* Hard Fault Handler */
    .long    MemManage_Handler     /* MPU Fault Handler */
    .long    BusFault_Handler      /* Bus Fault Handler */
    .long    UsageFault_Handler    /* Usage Fault Handler */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    SVC_Handler           /* SVCall Handler */
    .long    DebugMon_Handler      /* Debug Monitor Handler */
    .long    0                     /* Reserved */
    .long    PendSV_Handler        /* PendSV Handler */
    .long    SysTick_Handler       /* SysTick Handler */

    /* External interrupts of the STM32L432: weak aliases of Default_Handler */

    .macro    irq_handler    handler_name
    .weak     \handler_name
    .thumb_set \handler_name, Default_Handler
    .long     \handler_name
    .endm

    irq_handler WWDG_Handler                 /*  0 */
    irq_handler PVD_PVM_Handler              /*  1 */
    irq_handler TAMP_STAMP_Handler           /*  2 */
    irq_handler RTC_WKUP_Handler             /*  3 */
    irq_handler FLASH_Handler                /*  4 */
    irq_handler RCC_Handler                  /*  5 */
    irq_handler EXTI0_Handler                /*  6 */
    irq_handler EXTI1_Handler                /*  7 */
    irq_handler EXTI2_Handler                /*  8 */
    irq_handler EXTI3_Handler                /*  9 */
    irq_handler EXTI4_Handler                /* 10 */
    irq_handler DMA1_Channel1_Handler        /* 11 */
    irq_handler DMA1_Channel2_Handler        /* 12 */
    irq_handler DMA1_Channel3_Handler        /* 13 */
    irq_handler DMA1_Channel4_Handler        /* 14 */
    irq_handler DMA1_Channel5_Handler        /* 15 */
    irq_handler DMA1_Channel6_Handler        /* 16 */
    irq_handler DMA1_Channel7_Handler        /* 17 */
    irq_handler ADC1_Handler                 /* 18 */
    irq_handler CAN1_TX_Handler              /* 19 */
    irq_handler CAN1_RX0_Handler             /* 20 */
    irq_handler CAN1_RX1_Handler             /* 21 */
    irq_handler CAN1_SCE_Handler             /* 22 */
    irq_handler EXTI9_5_Handler              /* 23 */
    irq_handler TIM1_BRK_TIM15_Handler       /* 24 */
    irq_handler TIM1_UP_TIM16_Handler        /* 25 */
    irq_handler TIM1_TRG_COM_Handler         /* 26 */
    irq_handler TIM1_CC_Handler              /* 27 */
    irq_handler TIM2_Handler                 /* 28 */
    .long     0                              /* 29: reserved */
    .long     0                              /* 30: reserved */
    irq_handler I2C1_EV_Handler              /* 31 */
    irq_handler I2C1_ER_Handler              /* 32 */
    .long     0                              /* 33: reserved */
    .long     0                              /* 34: reserved */
    irq_handler SPI1_Handler                 /* 35 */
    .long     0                              /* 36: reserved */
    irq_handler USART1_Handler               /* 37 */
    irq_handler USART2_Handler               /* 38 */
    .long     0                              /* 39: reserved */
    irq_handler EXTI15_10_Handler            /* 40 */
    irq_handler RTC_Alarm_Handler            /* 41 */
    .long     0                              /* 42: reserved */
    .long     0                              /* 43: reserved */
    .long     0                              /* 44: reserved */
    .long     0                              /* 45: reserved */
    .long     0                              /* 46: reserved */
    .long     0                              /* 47: reserved */
    .long     0                              /* 48: reserved */
    .long     0                              /* 49: reserved */
    .long     0                              /* 50: reserved */
    irq_handler SPI3_Handler                 /* 51 */
    .long     0                              /* 52: reserved */
    .long     0                              /* 53: reserved */
    irq_handler TIM6_DAC_Handler             /* 54 */
    irq_handler TIM7_Handler                 /* 55 */
    irq_handler DMA2_Channel1_Handler        /* 56 */
    irq_handler DMA2_Channel2_Handler        /* 57 */
    irq_handler DMA2_Channel3_Handler        /* 58 */
    irq_handler DMA2_Channel4_Handler        /* 59 */
    irq_handler DMA2_Channel5_Handler        /* 60 */
    .long     0                              /* 61: reserved */
    .long     0                              /* 62: reserved */
    .long     0                              /* 63: reserved */
    irq_handler COMP_Handler                 /* 64 */
    irq_handler LPTIM1_Handler               /* 65 */
    irq_handler LPTIM2_Handler               /* 66 */
    irq_handler USB_Handler                  /* 67 */
    irq_handler DMA2_Channel6_Handler        /* 68 */
    irq_handler DMA2_Channel7_Handler        /* 69 */
    irq_handler LPUART1_Handler              /* 70 */
    irq_handler QUADSPI_Handler              /* 71 */
    irq_handler I2C3_EV_Handler              /* 72 */
    irq_handler I2C3_ER_Handler              /* 73 */
    irq_handler SAI1_Handler                 /* 74 */
    .long     0                              /* 75: reserved */
    irq_handler SWPMI1_Handler               /* 76 */
    irq_handler TSC_Handler                  /* 77 */
    .long     0                              /* 78: reserved */
    .long     0                              /* 79: reserved */
    irq_handler RNG_Handler                  /* 80 */
    irq_handler FPU_Handler                  /* 81 */
    irq_handler CRS_Handler                  /* 82 */

    .size    __isr_vector, . - __isr_vector

/************************************************************************************
 *
 *  Reset_Handler()
 *  This function gets called at start of execution after a reset event.
 *  Copies data from ROM to RAM, clears BSS if defined,
 *  calls SystemInit() if defined, finally calls main()
 ************************************************************************************/
    .text
    .thumb
    .global __stack_end__
    .thumb_func
    .align 2
    .globl    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:

/*  Setup stack pointer. Helpful for targets running in RAM without script file */
    ldr   r1, =__stack_end__
    msr   msp, r1

/*  Loop to copy data from read only memory to RAM. The ranges
 *   of copy from/to are specified by following symbols evaluated in
 *   linker script.
 *   __etext: End of code section, i.e., begin of data sections to copy from.
 *   __data_start__/__data_end__: RAM address range that data should be
 *   copied to. Both must be aligned to 4 bytes boundary.  */

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

#if 1
/*  Here are two copies of loop implemenations. First one favors code size
 *  and the second one favors performance. Default uses the first one.
 *  Change to "#if 0" to use the second one */
.LC0:
    cmp     r2, r3
    ittt    lt
    ldrlt   r0, [r1], #4
    strlt   r0, [r2], #4
    blt    .LC0
#else
    subs    r3, r2
    ble    .LC1
.LC0:
    subs    r3, #4
    ldr    r0, [r1, r3]
    str    r0, [r2, r3]
    bgt    .LC0
.LC1:
#endif

/*  Loop to zero out BSS section, which uses following symbols
 *  in linker script:
 *   __bss_start__: start of BSS section. Must align to 4
 *   __bss_end__: end of BSS section. Must align to 4
 */
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    movs    r0, 0
.LC2:
    cmp     r1, r2
    itt    lt
    strlt   r0, [r1], #4
    blt    .LC2

#ifdef __SYSTEM_INIT
    bl    SystemInit
#endif

    bl    main
    .pool
    .size Reset_Handler, . - Reset_Handler

/************************************************************************************
 *
 *  Weak definition for exceptions.
 *  Any function with the same name will override the weak definition.
 ************************************************************************************/
/*  Macro to define default handlers. Default handler
 *  will be weak symbol and just dead loops. They can be
 *  overwritten by other handlers */
    .macro    def_default_handler    handler_name
    .align 1
    .thumb_func
    .weak    \handler_name
    .type    \handler_name, %function
\handler_name :
    b    .
    .size    \handler_name, . - \handler_name
    .endm

    def_default_handler    NMI_Handler
    def_default_handler    HardFault_Handler
    def_default_handler    MemManage_Handler
    def_default_handler    BusFault_Handler
    def_default_handler    UsageFault_Handler
    def_default_handler    SVC_Handler
    def_default_handler    DebugMon_Handler
    def_default_handler    PendSV_Handler
    def_default_handler    SysTick_Handler
    def_default_handler    Default_Handler

    .weak    DEF_IRQHandler
    .set    DEF_IRQHandler, Default_Handler

    .end
