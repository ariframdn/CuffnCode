/**
 * @file    pressure.c
 * @brief   Algoritma oscillometric untuk pengukuran tekanan darah
 * @author  Tim CuffnCode
 *
 * @details Metode oscillometric:
 *          1. Ekstraksi envelope osilasi dari sinyal tekanan deflasi
 *          2. MAP = puncak envelope osilasi
 *          3. SBP = 0.55 * MAP amplitude (sisi tekanan tinggi)
 *          4. DBP = 0.75 * MAP amplitude (sisi tekanan rendah)
 *          5. HR = frekuensi osilasi
 */

#include "pressure.h"
#include "filter.h"
#include "uart.h"
#include <string.h>
#include <math.h>

/* ======================== Konstanta Internal ======================== */

#define ENVELOPE_SIZE       512
#define PEAK_WINDOW         20      /* Window deteksi puncak (sampel) */
#define MIN_PEAK_DISTANCE   30      /* Jarak minimal antar puncak */

/* ======================== Struktur Data Internal ======================== */

typedef struct {
    float pressure;
    float amplitude;
} EnvelopePoint_t;

/* ======================== Variabel Internal ======================== */

static MeasurementResult_t results;
static bool is_measuring = false;

/* Sirkular buffer untuk data mentah */
#define RAW_BUFFER_SIZE     2000
static float raw_pressure_buffer[RAW_BUFFER_SIZE];
static float raw_osc_buffer[RAW_BUFFER_SIZE];
static uint32_t raw_timestamp[RAW_BUFFER_SIZE];
static volatile uint32_t raw_head = 0;

/* Envelope buffer */
static EnvelopePoint_t envelope[ENVELOPE_SIZE];
static int envelope_count = 0;

/* Variabel pemrosesan */
static float prev_filtered = 0.0f;
static float baseline = 0.0f;

/* ======================== Inisialisasi ======================== */

void Pressure_Init(void)
{
    Pressure_Reset();
}

void Pressure_Reset(void)
{
    memset(&results, 0, sizeof(results));
    memset(raw_pressure_buffer, 0, sizeof(raw_pressure_buffer));
    memset(raw_osc_buffer, 0, sizeof(raw_osc_buffer));
    memset(raw_timestamp, 0, sizeof(raw_timestamp));
    memset(envelope, 0, sizeof(envelope));

    raw_head = 0;
    envelope_count = 0;
    is_measuring = false;
    prev_filtered = 0.0f;
    baseline = 0.0f;
}

/* ======================== Manajemen ======================== */

void Pressure_StartMeasurement(void)
{
    Pressure_Reset();
    Filter_Reset();
    is_measuring = true;
}

void Pressure_StopMeasurement(void)
{
    is_measuring = false;
}

bool Pressure_IsMeasuring(void)
{
    return is_measuring;
}

/* ======================== Pemrosesan Sampel ======================== */

void Pressure_ProcessSample(float pressure_mmhg, uint32_t timestamp_ms)
{
    if (!is_measuring) return;

    /* Simpan ke raw buffer */
    uint32_t idx = raw_head % RAW_BUFFER_SIZE;
    raw_pressure_buffer[idx] = pressure_mmhg;
    raw_timestamp[idx] = timestamp_ms;
    raw_head++;

    /* Filter chain */
    float lp = Filter_LowPass(pressure_mmhg);           /* DC pressure */
    float osc = Filter_BandPass(pressure_mmhg);          /* Oscillation */
    osc = Filter_Notch(osc);                              /* Remove hum */
    raw_osc_buffer[idx] = osc;

    /* Deteksi baseline */
    if (raw_head < 100) {
        baseline += pressure_mmhg;
        if (raw_head == 99) {
            baseline /= 100.0f;
        }
        return;
    }

    /* Deteksi puncak osilasi */
    static int sample_count = 0;
    static float peak_value = 0.0f;
    static float current_window_max = 0.0f;

    sample_count++;
    float abs_osc = (osc < 0) ? -osc : osc;

    if (abs_osc > current_window_max) {
        current_window_max = abs_osc;
    }

    /* Setiap PEAK_WINDOW sampel, simpan ke envelope */
    if (sample_count >= PEAK_WINDOW) {
        if (current_window_max > MIN_OSCILLATION_AMPLITUDE) {
            if (envelope_count < ENVELOPE_SIZE) {
                envelope[envelope_count].pressure  = pressure_mmhg;
                envelope[envelope_count].amplitude = current_window_max;
                envelope_count++;
            }
        }
        current_window_max = 0.0f;
        sample_count = 0;
    }
}

/* ======================== Ekstraksi Envelope ======================== */

void Pressure_ExtractEnvelope(void)
{
    if (envelope_count < 10) return;

    /* Smooth envelope dengan moving average */
    float smoothed[ENVELOPE_SIZE];
    int window = 3;

    for (int i = 0; i < envelope_count; i++) {
        float sum = 0.0f;
        int count = 0;
        for (int j = -window; j <= window; j++) {
            int idx = i + j;
            if (idx >= 0 && idx < envelope_count) {
                sum += envelope[idx].amplitude;
                count++;
            }
        }
        smoothed[i] = sum / count;
    }

    /* Copy smoothed back */
    for (int i = 0; i < envelope_count; i++) {
        envelope[i].amplitude = smoothed[i];
    }
}

/* ======================== Kalkulasi Hasil ======================== */

void Pressure_CalculateResults(void)
{
    if (envelope_count < 10) {
        results.valid = false;
        return;
    }

    /* Ekstrak envelope */
    Pressure_ExtractEnvelope();

    /* Cari MAP: puncak envelope */
    float max_amplitude = 0.0f;
    int map_index = 0;

    for (int i = 0; i < envelope_count; i++) {
        if (envelope[i].amplitude > max_amplitude) {
            max_amplitude = envelope[i].amplitude;
            map_index = i;
        }
    }

    results.map = envelope[map_index].pressure;

    /* Cari SBP: pressure pada 55% amplitude MAX (sisi kiri / tekanan tinggi) */
    float sbp_threshold = max_amplitude * OSCILLOMETRIC_SBP_RATIO;
    results.systolic = 0;

    for (int i = map_index; i >= 0; i--) {
        if (envelope[i].amplitude <= sbp_threshold) {
            results.systolic = (uint16_t)envelope[i].pressure;
            break;
        }
    }
    if (results.systolic == 0) results.systolic = (uint16_t)(results.map + 20);

    /* Cari DBP: pressure pada 75% amplitude MAX (sisi kanan / tekanan rendah) */
    float dbp_threshold = max_amplitude * OSCILLOMETRIC_DBP_RATIO;
    results.diastolic = 0;

    for (int i = map_index; i < envelope_count; i++) {
        if (envelope[i].amplitude <= dbp_threshold) {
            results.diastolic = (uint16_t)envelope[i].pressure;
            break;
        }
    }
    if (results.diastolic == 0) results.diastolic = (uint16_t)(results.map - 10);

    /* Hitung Heart Rate dari osilasi */
    /* Cari zero-crossing dalam osilasi untuk estimasi periode */
    int peak_count = 0;
    uint32_t last_peak_time = 0;
    uint32_t total_period = 0;

    for (uint32_t i = 1; i < (raw_head < RAW_BUFFER_SIZE ? raw_head : RAW_BUFFER_SIZE); i++) {
        if (raw_osc_buffer[i] > 0.01f && raw_osc_buffer[i-1] <= 0.01f) {
            if (last_peak_time > 0) {
                total_period += raw_timestamp[i] - last_peak_time;
                peak_count++;
            }
            last_peak_time = raw_timestamp[i];
        }
    }

    if (peak_count > 0) {
        float avg_period_ms = (float)total_period / peak_count;
        if (avg_period_ms > 0) {
            results.heart_rate = (uint16_t)(60000.0f / avg_period_ms);
        }
    }

    /* Validasi hasil */
    results.valid = (results.systolic > 0 && results.diastolic > 0 &&
                     results.systolic > results.diastolic &&
                     results.systolic < 300 && results.diastolic < 200 &&
                     results.heart_rate > 30 && results.heart_rate < 220);
}

/* ======================== Getter ======================== */

MeasurementResult_t Pressure_GetResults(void)
{
    return results;
}

float Pressure_GetCurrentPressure(void)
{
    if (raw_head == 0) return 0.0f;
    uint32_t last = (raw_head - 1) % RAW_BUFFER_SIZE;
    return raw_pressure_buffer[last];
}

float Pressure_GetOscillation(void)
{
    if (raw_head == 0) return 0.0f;
    uint32_t last = (raw_head - 1) % RAW_BUFFER_SIZE;
    return raw_osc_buffer[last];
}

/* ======================== Dump Data ======================== */

void Pressure_DumpRawData(void)
{
    uint32_t count = (raw_head < RAW_BUFFER_SIZE) ? raw_head : RAW_BUFFER_SIZE;

    UART_SendCSVHeader();

    for (uint32_t i = 0; i < count; i++) {
        UART_SendCSVData(raw_timestamp[i], raw_pressure_buffer[i],
                         raw_osc_buffer[i], 0);
    }

    UART_Printf("\r\n[Dump] %lu samples dumped.\r\n", count);
}
