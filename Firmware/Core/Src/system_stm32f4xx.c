/**
 * @file    system_stm32f4xx.c
 * @brief   System initialization untuk STM32F411CE
 * @author  Tim CuffnCode
 *
 * @details Menyediakan SystemInit() yang dipanggil oleh startup
 *          sebelum main(). Mengatur clock, FPU, dan interrupt.
 */

#include <stdint.h>

/* ======================== External Oscillator ======================== */
#define HSE_VALUE               25000000U  /* 25 MHz crystal on Black Pill */

/* ======================== System Core Clock ======================== */
uint32_t SystemCoreClock = 16000000U;      /* Default HSI clock */

/* ======================== SystemInit ======================== */

/**
 * @brief  Inisialisasi sistem mikrocontroller
 * @note   Setup FPU, vector table offset, dan clock
 *         Konfigurasi clock detail ada di SystemClock_Config (main.c)
 */
void SystemInit(void)
{
    /* === FPU Enable === */
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
        /* CPACR: Coprocessor Access Control Register */
        /* Set CP10 and CP11 Full Access (bits 20-23) */
        uint32_t cpacr = *(volatile uint32_t *)0xE000ED88;
        cpacr |= (0xF << 20);
        *(volatile uint32_t *)0xE000ED88 = cpacr;
    #endif

    /* === Vector Table Offset === */
    /* Default: 0x08000000 (start of Flash) */
    extern uint32_t g_pfnVectors;
    SCB->VTOR = (uint32_t)&g_pfnVectors;

    /* === Configure Flash Prefetch, Instruction Cache, Data Cache === */
    /* FLASH_ACR: PRFTEN=1, ICEN=1, DCEN=1, LATENCY=5WS (100MHz) */
    #if defined(FLASH_ACR_LATENCY_5WS)
        FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;
    #else
        FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_4WS;
    #endif
}

/* ======================== SystemCoreClockUpdate ======================== */

/**
 * @brief  Update SystemCoreClock variable
 * @note   Dipanggil setelah clock configuration berubah
 */
void SystemCoreClockUpdate(void)
{
    /* Read RCC_CFGR and determine system clock source */
    uint32_t src = (RCC->CFGR & RCC_CFGR_SWS_Msk) >> RCC_CFGR_SWS_Pos;

    switch (src)
    {
        case 0x00:  /* HSI */
            SystemCoreClock = 16000000U;
            break;

        case 0x01:  /* HSE */
            SystemCoreClock = HSE_VALUE;
            break;

        case 0x02:  /* PLL */
            /* Simplified: assume PLL = 100MHz */
            SystemCoreClock = 100000000U;
            break;

        default:
            SystemCoreClock = 16000000U;
            break;
    }
}
