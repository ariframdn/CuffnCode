/**
 * @file    adc.c
 * @brief   Driver ADC STM32F411CE untuk sensor tekanan
 * @author  Tim CuffnCode
 *
 * @details ADC1 channel 0 (PA0) membaca output AFE.
 *          Trigger dari TIM2 @ 1kHz. DMA circular double buffer.
 *          Kalibrasi offset dan konversi ke mmHg.
 */

#include "adc.h"
#include "main.h"
#include <math.h>

/* ======================== Variabel Global ======================== */
static volatile uint16_t last_raw_value = 0;
static volatile float    last_voltage   = 0.0f;
static volatile float    last_pressure  = 0.0f;

/* Parameter kalibrasi */
static float cal_offset = 0.0f;     /* Offset tegangan (V) */
static float cal_gain   = 1.0f;     /* Faktor gain kalibrasi */
static float v_offset   = 1.5f;     /* Offset dari TLC2272 (V) */
static float v_max      = 3.3f;     /* Tegangan maksimum ADC (V) */
static float p_max      = 300.0f;   /* Tekanan maksimum (mmHg) */

/* Sensor MPS20N0040D: full-scale ~50-100mV
 * AFE gain AD620: ~105x
 * Output range: 1.5V + (0.1V * 105) = 1.5V + 10.5V -> clamped to 3.3V
 * Untuk 300mmHg: V_out ≈ 1.5V + (pressure/300) * (V_max - V_offset)
 * Maka: pressure = (V_out - V_offset) * p_max / (V_max - V_offset)
 */

/* ======================== Inisialisasi ======================== */

void ADC_Init(void)
{
    /* ADC init dilakukan di main.c via HAL */
    /* Di sini hanya set parameter kalibrasi default */
    cal_offset = 0.0f;
    cal_gain   = 1.0f;
}

void ADC_Calibrate(void)
{
    /* Baca beberapa sampel saat tidak ada tekanan */
    uint32_t sum = 0;
    uint32_t samples = 100;

    for (uint32_t i = 0; i < samples; i++) {
        /* Trigger konversi single */
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            sum += HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(1);
    }

    float avg_voltage = ((float)sum / samples) * ADC_VREF / ADC_RESOLUTION;
    cal_offset = avg_voltage - v_offset;

    /* Kalibrasi gain dengan mengukur tegangan referensi */
    /* Gain ideal: p_max / (v_max - v_offset) */
    /* Actual gain perlu diukur dengan pressure reference */
    cal_gain = 1.0f; /* Default, perlu kalibrasi manual */
}

/* ======================== Pembacaan ======================== */

uint16_t ADC_GetRawValue(void)
{
    return last_raw_value;
}

float ADC_GetVoltage(void)
{
    return last_voltage;
}

float ADC_GetPressure_mmHg(void)
{
    return last_pressure;
}

/* ======================== Sampling Control ======================== */

void ADC_StartSampling(void)
{
    /* Mulai ADC dengan DMA */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, BUFFER_SIZE * 2);
}

void ADC_StopSampling(void)
{
    HAL_ADC_Stop_DMA(&hadc1);
}

/* ======================== Callback dari DMA ======================== */

/**
 * @brief  Update nilai terbaru dari DMA buffer
 * @param  raw: Nilai ADC mentah
 */
void ADC_UpdateValue(uint16_t raw)
{
    last_raw_value = raw;
    last_voltage = ((float)raw * ADC_VREF / ADC_RESOLUTION) - cal_offset;
    last_voltage *= cal_gain;

    /* Konversi ke mmHg */
    if (last_voltage <= v_offset) {
        last_pressure = 0.0f;
    } else if (last_voltage >= v_max) {
        last_pressure = p_max;
    } else {
        last_pressure = (last_voltage - v_offset) * p_max / (v_max - v_offset);
    }
}
