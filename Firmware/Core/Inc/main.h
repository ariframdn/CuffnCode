/**
 * @file    main.h
 * @brief   Header utama untuk firmware CuffnCode
 * @author  Tim CuffnCode
 * @date    2026
 *
 * @details Proyek CuffnCode - Retrofitted Blood Pressure Measurement System
 *          Menggunakan STM32F411CE (Black Pill) sebagai digital controller.
 *          Dana: IFAC Activity Fund (July 2025 - June 2026)
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Includes ======================== */
#include <stdint.h>
#include <stdbool.h>

/* ======================== Definisi Pin ======================== */

/* --- ADC --- */
#define ADC_PRESSURE_CHANNEL    ADC_CHANNEL_0   /* PA0 - Input tekanan dari AFE */
#define ADC_REFERENCE_CHANNEL   ADC_CHANNEL_1   /* PA1 - Tegangan referensi */

/* --- Pump (Pompa) --- */
#define PUMP_GPIO_PORT          GPIOA
#define PUMP_PIN                GPIO_PIN_8      /* PA8 - Kontrol pompa (MOSFET) */
#define PUMP_ON()               HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_PIN, GPIO_PIN_SET)
#define PUMP_OFF()              HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_PIN, GPIO_PIN_RESET)

/* --- Valve (Katup) --- */
#define VALVE_GPIO_PORT         GPIOB
#define VALVE_PIN               GPIO_PIN_0      /* PB0 - Kontrol katup solenoid */
#define VALVE_OPEN()            HAL_GPIO_WritePin(VALVE_GPIO_PORT, VALVE_PIN, GPIO_PIN_SET)
#define VALVE_CLOSE()           HAL_GPIO_WritePin(VALVE_GPIO_PORT, VALVE_PIN, GPIO_PIN_RESET)

/* --- UART --- */
#define UART_DEBUG              USART2          /* USART2 (PA2=TX, PA3=RX) - Debug console */

/* --- LED Indikator --- */
#define LED_PC13_GPIO_PORT      GPIOC
#define LED_PC13_PIN            GPIO_PIN_13     /* PC13 - LED built-in Black Pill */
#define LED_ON()                HAL_GPIO_WritePin(LED_PC13_GPIO_PORT, LED_PC13_PIN, GPIO_PIN_RESET)
#define LED_OFF()               HAL_GPIO_WritePin(LED_PC13_GPIO_PORT, LED_PC13_PIN, GPIO_PIN_SET)
#define LED_TOGGLE()            HAL_GPIO_TogglePin(LED_PC13_GPIO_PORT, LED_PC13_PIN)

/* ======================== Konstanta Sistem ======================== */

#define SYS_CLOCK_HZ            100000000UL     /* 100 MHz (maks STM32F411) */
#define ADC_RESOLUTION          4095            /* 12-bit ADC */
#define ADC_VREF                3.3f            /* Tegangan referensi ADC (V) */
#define SAMPLE_RATE_HZ          1000            /* Frekuensi sampling (Hz) */
#define BUFFER_SIZE             1024            /* Ukuran buffer sirkular */

/* Parameter pengukuran tekanan darah */
#define CUFF_TARGET_PRESSURE    180             /* Tekanan target inflasi (mmHg) */
#define MAX_PUMP_TIME_MS        30000           /* Maks waktu pompa (30 detik) */
#define MEASUREMENT_TIMEOUT_MS  60000           /* Timeout pengukuran (60 detik) */
#define PRESSURE_SLOPE_THRESHOLD 3              /* Threshold slope untuk deteksi SBP/DBP */

/* ======================== State Machine ======================== */

typedef enum {
    STATE_IDLE = 0,             /* Mode standby */
    STATE_INFLATING,            /* Pompa aktif, cuff mengembang */
    STATE_MEASURING,            /* Katup terbuka, mengukur tekanan */
    STATE_COMPLETE,             /* Pengukuran selesai, tampilkan hasil */
    STATE_ERROR,                /* Terjadi error */
    STATE_DUMP                  /* Dump data mentah ke serial */
} SystemState_t;

/* ======================== Struktur Data ======================== */

typedef struct {
    uint16_t    raw_adc;            /* Nilai ADC mentah */
    float       voltage;            /* Tegangan terukur (V) */
    float       pressure_mmhg;      /* Tekanan dalam mmHg */
    uint32_t    timestamp_ms;       /* Timestamp sampling (ms) */
} PressureSample_t;

typedef struct {
    uint16_t    systolic;           /* Tekanan sistolik (mmHg) */
    uint16_t    diastolic;          /* Tekanan diastolik (mmHg) */
    uint16_t    heart_rate;         /* Denyut jantung (bpm) */
    float       map;                /* Mean Arterial Pressure (mmHg) */
    bool        valid;              /* Flag validitas hasil */
} MeasurementResult_t;

typedef struct {
    PressureSample_t    buffer[BUFFER_SIZE];
    volatile uint32_t   head;
    volatile uint32_t   tail;
    volatile uint32_t   count;
} CircularBuffer_t;

/* ======================== Fungsi Global ======================== */

void SystemClock_Config(void);
void GPIO_Init(void);
void ADC_Init(void);
void UART_Init(void);
void TIM_Init(void);
void Error_Handler(void);

/* Callback dari ADC */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
