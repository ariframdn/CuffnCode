/**
 * @file    startup_stm32f411xe.s
 * @brief   Startup file untuk STM32F411CE (Cortex-M4F)
 * @author  Tim CuffnCode
 *
 * @details Inisialisasi stack, heap, vector table, dan
 *          memanggil SystemInit() lalu main().
 *          Cocok untuk STM32F411xE (512KB Flash, 128KB SRAM).
 */

.syntax  unified
.cpu     cortex-m4
.fpu     spv5-d16
.thumb

.global g_pfnVectors
.global Default_Handler

/* ======================== Stack & Heap ======================== */
.equ  _estack,     0x20020000            /* End of 128KB SRAM */
.equ  _Min_Heap_Size, 0x200              /* 512 bytes */
.equ  _Min_Stack_Size, 0x400             /* 1KB */

/* ======================== Vector Table ======================== */
.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object
.size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* External Interrupts */
    .word WWDG_IRQHandler               /* Window WatchDog */
    .word PVD_IRQHandler                /* PVD through EXTI */
    .word TAMP_STAMP_IRQHandler         /* Tamper and TimeStamp */
    .word RTC_WKUP_IRQHandler           /* RTC Wakeup */
    .word FLASH_IRQHandler              /* Flash */
    .word RCC_IRQHandler                /* RCC */
    .word EXTI0_IRQHandler              /* EXTI Line0 */
    .word EXTI1_IRQHandler              /* EXTI Line1 */
    .word EXTI2_IRQHandler              /* EXTI Line2 */
    .word EXTI3_IRQHandler              /* EXTI Line3 */
    .word EXTI4_IRQHandler              /* EXTI Line4 */
    .word DMA1_Stream0_IRQHandler       /* DMA1 Stream0 */
    .word DMA1_Stream1_IRQHandler       /* DMA1 Stream1 */
    .word DMA1_Stream2_IRQHandler       /* DMA1 Stream2 */
    .word DMA1_Stream3_IRQHandler       /* DMA1 Stream3 */
    .word DMA1_Stream4_IRQHandler       /* DMA1 Stream4 */
    .word DMA1_Stream5_IRQHandler       /* DMA1 Stream5 */
    .word DMA1_Stream6_IRQHandler       /* DMA1 Stream6 */
    .word ADC_IRQHandler                /* ADC1 */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word EXTI9_5_IRQHandler            /* EXTI Line9..5 */
    .word TIM1_BRK_TIM9_IRQHandler      /* TIM1 Break and TIM9 */
    .word TIM1_UP_TIM10_IRQHandler      /* TIM1 Update and TIM10 */
    .word TIM1_TRG_COM_TIM11_IRQHandler /* TIM1 Trigger/Comm and TIM11 */
    .word TIM1_CC_IRQHandler            /* TIM1 Capture Compare */
    .word TIM2_IRQHandler               /* TIM2 */
    .word TIM3_IRQHandler               /* TIM3 */
    .word TIM4_IRQHandler               /* TIM4 */
    .word I2C1_EV_IRQHandler            /* I2C1 Event */
    .word I2C1_ER_IRQHandler            /* I2C1 Error */
    .word I2C2_EV_IRQHandler            /* I2C2 Event */
    .word I2C2_ER_IRQHandler            /* I2C2 Error */
    .word SPI1_IRQHandler               /* SPI1 */
    .word SPI2_IRQHandler               /* SPI2 */
    .word USART1_IRQHandler             /* USART1 */
    .word USART2_IRQHandler             /* USART2 */
    .word USART3_IRQHandler             /* USART3 */
    .word EXTI15_10_IRQHandler          /* EXTI Line15..10 */
    .word RTC_Alarm_IRQHandler          /* RTC Alarm A & B */
    .word OTG_FS_WKUP_IRQHandler        /* USB OTG FS Wakeup */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word DMA1_Stream7_IRQHandler       /* DMA1 Stream7 */
    .word 0                             /* Reserved */
    .word SDIO_IRQHandler               /* SDIO */
    .word TIM5_IRQHandler               /* TIM5 */
    .word SPI3_IRQHandler               /* SPI3 */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word DMA2_Stream0_IRQHandler       /* DMA2 Stream0 */
    .word DMA2_Stream1_IRQHandler       /* DMA2 Stream1 */
    .word DMA2_Stream2_IRQHandler       /* DMA2 Stream2 */
    .word DMA2_Stream3_IRQHandler       /* DMA2 Stream3 */
    .word DMA2_Stream4_IRQHandler       /* DMA2 Stream4 */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word OTG_FS_IRQHandler             /* USB OTG FS */
    .word DMA2_Stream5_IRQHandler       /* DMA2 Stream5 */
    .word DMA2_Stream6_IRQHandler       /* DMA2 Stream6 */
    .word DMA2_Stream7_IRQHandler       /* DMA2 Stream7 */
    .word USART6_IRQHandler             /* USART6 */
    .word I2C3_EV_IRQHandler            /* I2C3 Event */
    .word I2C3_ER_IRQHandler            /* I2C3 Error */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word SPI4_IRQHandler               /* SPI4 */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word FMC_IRQHandler                /* FMC */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word SDMMC1_IRQHandler             /* SDMMC1 */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */
    .word 0                             /* Reserved */

/* ======================== Reset Handler ======================== */
.section .text.Reset_Handler, "ax", %progbits
.type Reset_Handler, %function
Reset_Handler:
    /* Set stack pointer */
    ldr     sp, =_estack

    /* Copy .data section from Flash to SRAM */
    ldr     r1, =__etext
    ldr     r2, =__data_start__
    ldr     r3, =__data_end__

    subs    r3, r2
    ble     .L_loop1_done

.L_loop1:
    subs    r3, #4
    ldr     r0, [r1, r3]
    str     r0, [r2, r3]
    bgt     .L_loop1

.L_loop1_done:

    /* Zero-fill .bss section */
    ldr     r1, =__bss_start__
    ldr     r2, =__bss_end__
    movs    r0, 0

    subs    r3, r2, r1
    ble     .L_loop2_done

.L_loop2:
    subs    r3, #4
    str     r0, [r1, r3]
    bgt     .L_loop2

.L_loop2_done:

    /* Call SystemInit */
    bl      SystemInit

    /* Call C++ constructors (if any) */
    bl      __libc_init_array

    /* Call main() */
    bl      main

    /* Infinite loop if main returns */
.L_Infinite_Loop:
    b       .L_Infinite_Loop
.size Reset_Handler, .-Reset_Handler

/* ======================== Default Handler ======================== */
.section .text.Default_Handler, "ax", %progbits
.type Default_Handler, %function
Default_Handler:
    b       Default_Handler
.size Default_Handler, .-Default_Handler

/* ======================== Weak Handler Aliases ======================== */
.macro  def_irq_handler  handler_name
.weak   \handler_name
.set    \handler_name, Default_Handler
.endm

def_irq_handler  NMI_Handler
def_irq_handler  HardFault_Handler
def_irq_handler  MemManage_Handler
def_irq_handler  BusFault_Handler
def_irq_handler  UsageFault_Handler
def_irq_handler  SVC_Handler
def_irq_handler  DebugMon_Handler
def_irq_handler  PendSV_Handler
def_irq_handler  SysTick_Handler

def_irq_handler  WWDG_IRQHandler
def_irq_handler  PVD_IRQHandler
def_irq_handler  TAMP_STAMP_IRQHandler
def_irq_handler  RTC_WKUP_IRQHandler
def_irq_handler  FLASH_IRQHandler
def_irq_handler  RCC_IRQHandler
def_irq_handler  EXTI0_IRQHandler
def_irq_handler  EXTI1_IRQHandler
def_irq_handler  EXTI2_IRQHandler
def_irq_handler  EXTI3_IRQHandler
def_irq_handler  EXTI4_IRQHandler
def_irq_handler  DMA1_Stream0_IRQHandler
def_irq_handler  DMA1_Stream1_IRQHandler
def_irq_handler  DMA1_Stream2_IRQHandler
def_irq_handler  DMA1_Stream3_IRQHandler
def_irq_handler  DMA1_Stream4_IRQHandler
def_irq_handler  DMA1_Stream5_IRQHandler
def_irq_handler  DMA1_Stream6_IRQHandler
def_irq_handler  ADC_IRQHandler
def_irq_handler  EXTI9_5_IRQHandler
def_irq_handler  TIM1_BRK_TIM9_IRQHandler
def_irq_handler  TIM1_UP_TIM10_IRQHandler
def_irq_handler  TIM1_TRG_COM_TIM11_IRQHandler
def_irq_handler  TIM1_CC_IRQHandler
def_irq_handler  TIM2_IRQHandler
def_irq_handler  TIM3_IRQHandler
def_irq_handler  TIM4_IRQHandler
def_irq_handler  I2C1_EV_IRQHandler
def_irq_handler  I2C1_ER_IRQHandler
def_irq_handler  I2C2_EV_IRQHandler
def_irq_handler  I2C2_ER_IRQHandler
def_irq_handler  SPI1_IRQHandler
def_irq_handler  SPI2_IRQHandler
def_irq_handler  USART1_IRQHandler
def_irq_handler  USART2_IRQHandler
def_irq_handler  USART3_IRQHandler
def_irq_handler  EXTI15_10_IRQHandler
def_irq_handler  RTC_Alarm_IRQHandler
def_irq_handler  OTG_FS_WKUP_IRQHandler
def_irq_handler  DMA1_Stream7_IRQHandler
def_irq_handler  SDIO_IRQHandler
def_irq_handler  TIM5_IRQHandler
def_irq_handler  SPI3_IRQHandler
def_irq_handler  DMA2_Stream0_IRQHandler
def_irq_handler  DMA2_Stream1_IRQHandler
def_irq_handler  DMA2_Stream2_IRQHandler
def_irq_handler  DMA2_Stream3_IRQHandler
def_irq_handler  DMA2_Stream4_IRQHandler
def_irq_handler  OTG_FS_IRQHandler
def_irq_handler  DMA2_Stream5_IRQHandler
def_irq_handler  DMA2_Stream6_IRQHandler
def_irq_handler  DMA2_Stream7_IRQHandler
def_irq_handler  USART6_IRQHandler
def_irq_handler  I2C3_EV_IRQHandler
def_irq_handler  I2C3_ER_IRQHandler
def_irq_handler  SPI4_IRQHandler
def_irq_handler  FMC_IRQHandler
def_irq_handler  SDMMC1_IRQHandler

/* ======================== End ======================== */
.end
