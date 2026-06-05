/**
 * @file    main.c
 * @brief   Firmware utama CuffnCode - STM32F411CE (Black Pill)
 * @author  Tim CuffnCode
 * @date    2026
 *
 * @details Retrofitted Blood Pressure Measurement System.
 *          State machine: IDLE -> INFLATING -> MEASURING -> COMPLETE
 *          Dengan sampling ADC 1kHz, filter digital, algoritma oscillometric.
 */

/* ======================== Includes ======================== */
#include "main.h"
#include "adc.h"
#include "pump.h"
#include "valve.h"
#include "filter.h"
#include "pressure.h"
#include "uart.h"

#include <string.h>
#include <stdio.h>

/* ======================== HAL Handles ======================== */
ADC_HandleTypeDef       hadc1;
DMA_HandleTypeDef       hdma_adc1;
UART_HandleTypeDef      huart2;
TIM_HandleTypeDef       htim2;

/* ======================== Global Variables ======================== */
static SystemState_t    system_state    = STATE_IDLE;
static uint32_t         tick_ms         = 0;
static uint32_t         state_timer     = 0;
static uint16_t         adc_buffer[2][BUFFER_SIZE];
static volatile uint8_t adc_dma_active_buffer = 0;
static volatile uint8_t adc_dma_half_complete  = 0;
static volatile uint8_t adc_dma_full_complete  = 0;

/* UART RX */
static uint8_t          uart_rx_byte;
static volatile uint8_t uart_cmd_received = 0;
static char             uart_cmd;

/* ======================== System Clock Configuration ======================== */

/**
 * @brief  Konfigurasi system clock STM32F411CE
 * @note   HSE 25MHz -> PLL -> SYSCLK 100MHz
 *         APB1 = 50MHz, APB2 = 100MHz
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /* HSE oscillator configuration */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;       /* 25MHz / 25 = 1MHz */
    RCC_OscInitStruct.PLL.PLLN = 200;      /* 1MHz * 200 = 200MHz */
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  /* 200MHz / 2 = 100MHz */
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* SYSCLK, HCLK, PCLK configuration */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;      /* HCLK = 100MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;       /* APB1 = 50MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;       /* APB2 = 100MHz */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }

    /* Configure peripherals clocks */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }

    SystemCoreClockUpdate();
}

/* ======================== GPIO Initialization ======================== */

/**
 * @brief  Inisialisasi GPIO
 * @note   PA0 = ADC input (analog)
 *         PA8 = Pump control (push-pull output)
 *         PB0 = Valve control (push-pull output)
 *         PC13 = LED (push-pull output)
 */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* PA0, PA1: Analog mode untuk ADC */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA2: USART2_TX, PA3: USART2_RX */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA8: Pump control */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

    /* PB0: Valve control */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    /* PC13: LED indikator */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); /* LED OFF (active low) */
}

/* ======================== ADC Initialization ======================== */

void ADC_Init(void)
{
    /* ADC1 init */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    HAL_ADC_Init(&hadc1);

    /* Configure ADC channel 0 (PA0) */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/* ======================== UART Initialization ======================== */

void UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = UART_BAUDRATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);

    /* Enable RX interrupt */
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
}

/* ======================== Timer Initialization ======================== */

void TIM_Init(void)
{
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 100 - 1;        /* 100MHz / 100 = 1MHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1000 - 1;           /* 1MHz / 1000 = 1kHz */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim2);

    /* Select TIM2 as master for trigger output */
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
}

/* ======================== DMA Initialization ======================== */

static void DMA_Init(void)
{
    /* DMA2 clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA2 Stream0 configuration for ADC1 */
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc1);

    /* Link DMA to ADC */
    __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

    /* Enable DMA interrupt */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

/* ======================== HAL MSP Initialization ======================== */

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        __HAL_RCC_ADC1_CLK_ENABLE();
        HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ADC_IRQn);
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART2) {
        __HAL_RCC_USART2_CLK_ENABLE();
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

/* ======================== Interrupt Handlers ======================== */

void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}

void USART2_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) {
        uart_cmd = (char)(huart2.Instance->DR & 0xFF);
        uart_cmd_received = 1;
    }
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void SysTick_Handler(void)
{
    tick_ms++;
}

/* ======================== ADC DMA Callbacks ======================== */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        adc_dma_full_complete = 1;
        adc_dma_active_buffer = (adc_dma_active_buffer + 1) % 2;
    }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) {
        adc_dma_half_complete = 1;
    }
}

/* ======================== Error Handler ======================== */

void Error_Handler(void)
{
    /* Disable pump, open valve, LED alarm */
    Pump_Off();
    Valve_Open();

    UART_Printf("[ERROR] System error! Emergency deflation active.\r\n");

    /* LED blink cepat (SOS pattern) */
    while (1) {
        for (int i = 0; i < 3; i++) { LED_ON();  HAL_Delay(150); LED_OFF(); HAL_Delay(150); }
        HAL_Delay(500);
        for (int i = 0; i < 3; i++) { LED_ON();  HAL_Delay(350); LED_OFF(); HAL_Delay(350); }
        HAL_Delay(500);
        for (int i = 0; i < 3; i++) { LED_ON();  HAL_Delay(150); LED_OFF(); HAL_Delay(150); }
        HAL_Delay(2000);
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    UART_Printf("[ASSERT] Assert failed: file %s, line %lu\r\n", file, line);
    Error_Handler();
}
#endif

/* ======================== UART Command Processing ======================== */

static void ProcessUARTCommand(char cmd)
{
    switch (cmd) {
        case 's':
        case 'S':
            if (system_state == STATE_IDLE) {
                system_state = STATE_INFLATING;
                state_timer = tick_ms;
                UART_Printf("[CMD] Starting measurement...\r\n");
            } else {
                UART_Printf("[CMD] System busy (state=%d)\r\n", system_state);
            }
            break;

        case 'd':
        case 'D':
            if (system_state == STATE_IDLE) {
                system_state = STATE_DUMP;
                UART_Printf("[CMD] Dumping raw data...\r\n");
            } else {
                UART_Printf("[CMD] Cannot dump while busy\r\n");
            }
            break;

        case 'r':
        case 'R':
            UART_Printf("[CMD] Resetting system...\r\n");
            Pressure_Reset();
            Pump_Off();
            Valve_Close();
            system_state = STATE_IDLE;
            break;

        case 'h':
        case 'H':
            UART_Printf("\r\n=== CuffnCode Commands ===\r\n");
            UART_Printf("  s - Start measurement\r\n");
            UART_Printf("  d - Dump raw data (CSV)\r\n");
            UART_Printf("  r - Reset system\r\n");
            UART_Printf("  h - This help\r\n");
            UART_Printf("==========================\r\n\r\n");
            break;

        default:
            UART_Printf("[CMD] Unknown: '%c'. Send 'h' for help.\r\n", cmd);
            break;
    }
}

/* ======================== State Machine Processing ======================== */

static void ProcessStateMachine(void)
{
    uint32_t elapsed = tick_ms - state_timer;
    float current_pressure = Pressure_GetCurrentPressure();

    switch (system_state) {
        case STATE_IDLE:
            /* LED slow blink (1Hz) */
            if ((tick_ms % 1000) < 500) {
                LED_OFF();
            } else {
                LED_ON();
            }
            /* Proses command via UART done in interrupt */
            break;

        case STATE_INFLATING:
            LED_ON(); /* LED ON selama pumping */
            UART_Printf("[STATE] Inflating... pressure=%.1f mmHg\r", current_pressure);

            if (Pump_Process((uint16_t)current_pressure)) {
                /* Target reached, mulai measuring */
                Pump_Off();
                Valve_Open();
                Pressure_StartMeasurement();
                system_state = STATE_MEASURING;
                state_timer = tick_ms;
                UART_Printf("\r\n[STATE] Target reached! Starting measurement...\r\n");
            }

            /* Timeout check */
            if (elapsed > MAX_PUMP_TIME_MS) {
                UART_Printf("\r\n[ERROR] Pump timeout!\r\n");
                Pump_Off();
                Valve_QuickDump();
                system_state = STATE_ERROR;
            }
            break;

        case STATE_MEASURING:
            /* Controlled deflation + sampling */
            if (Valve_ProcessDeflation(elapsed)) {
                /* Deflasi selesai */
                Pressure_CalculateResults();
                system_state = STATE_COMPLETE;
                UART_Printf("\r\n[STATE] Measurement complete!\r\n");
            }

            /* Timeout check */
            if (elapsed > MEASUREMENT_TIMEOUT_MS) {
                UART_Printf("\r\n[ERROR] Measurement timeout!\r\n");
                Valve_QuickDump();
                system_state = STATE_ERROR;
            }
            break;

        case STATE_COMPLETE:
            Valve_QuickDump();

            /* Kirim hasil via UART */
            MeasurementResult_t result = Pressure_GetResults();
            UART_SendJSONResult(result);

            /* Kirim juga CSV header + summary */
            UART_Printf("\r\n=== Measurement Results ===\r\n");
            UART_Printf("Systolic:   %u mmHg\r\n", result.systolic);
            UART_Printf("Diastolic:  %u mmHg\r\n", result.diastolic);
            UART_Printf("MAP:        %.1f mmHg\r\n", result.map);
            UART_Printf("Heart Rate: %u bpm\r\n", result.heart_rate);
            UART_Printf("Valid:      %s\r\n", result.valid ? "Yes" : "No");
            UART_Printf("============================\r\n\r\n");

            /* LED flash 3x untuk indikasi selesai */
            for (int i = 0; i < 3; i++) {
                LED_ON();  HAL_Delay(100);
                LED_OFF(); HAL_Delay(100);
            }

            system_state = STATE_IDLE;
            break;

        case STATE_ERROR:
            Pump_Off();
            Valve_QuickDump();
            /* LED fast blink */
            LED_TOGGLE();
            HAL_Delay(200);

            /* Tunggu reset command */
            if (uart_cmd_received && (uart_cmd == 'r' || uart_cmd == 'R')) {
                Pressure_Reset();
                system_state = STATE_IDLE;
                uart_cmd_received = 0;
                UART_Printf("[STATE] System reset from error.\r\n");
            }
            break;

        case STATE_DUMP:
            Pressure_DumpRawData();
            UART_Printf("[STATE] Dump complete.\r\n");
            system_state = STATE_IDLE;
            break;

        default:
            system_state = STATE_IDLE;
            break;
    }
}

/* ======================== Main Function ======================== */

int main(void)
{
    /* === Inisialisasi Hardware === */
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();

    DMA_Init();
    ADC_Init();
    UART_Init();
    TIM_Init();

    /* === Inisialisasi Modul === */
    Pump_Init();
    Valve_Init();
    Filter_Init(SAMPLE_RATE_HZ);
    Pressure_Init();

    /* === Start ADC Sampling === */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, BUFFER_SIZE * 2);

    /* === Start Timer === */
    HAL_TIM_Base_Start(&htim2);

    /* === UART Welcome Message === */
    UART_Printf("\r\n");
    UART_Printf("===================================\r\n");
    UART_Printf("  CuffnCode v1.0\r\n");
    UART_Printf("  Retrofitted Blood Pressure Monitor\r\n");
    UART_Printf("  IFAC Activity Fund 2025-2026\r\n");
    UART_Printf("===================================\r\n");
    UART_Printf("System ready. Send 'h' for help.\r\n");
    UART_Printf("> ");

    /* === Main Loop === */
    while (1)
    {
        /* Process UART commands */
        if (uart_cmd_received) {
            uart_cmd_received = 0;
            ProcessUARTCommand(uart_cmd);
            if (system_state == STATE_IDLE) {
                UART_Printf("\r\n> ");
            }
        }

        /* Process state machine (dipanggil setiap iterasi loop) */
        ProcessStateMachine();

        /* ADC data processing */
        if (adc_dma_half_complete || adc_dma_full_complete) {
            uint32_t start_idx = adc_dma_half_complete ? 0 : BUFFER_SIZE;
            adc_dma_half_complete = 0;
            adc_dma_full_complete = 0;

            /* Proses buffer yang baru terisi */
            uint8_t buf_idx = (adc_dma_active_buffer == 0) ? 1 : 0;
            uint32_t offset = (buf_idx == 0) ? 0 : BUFFER_SIZE;

            for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
                uint16_t raw = adc_buffer[buf_idx][i];
                float voltage = (float)raw * ADC_VREF / ADC_RESOLUTION;
                float pressure = (voltage - 1.5f) * 300.0f / (3.3f - 1.5f);
                if (pressure < 0) pressure = 0;

                Pressure_ProcessSample(pressure, tick_ms);
            }
        }
    }
}
