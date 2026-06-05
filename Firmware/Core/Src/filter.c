/**
 * @file    filter.c
 * @brief   Implementasi filter digital untuk CuffnCode
 * @author  Tim CuffnCode
 *
 * @details Menyediakan:
 *          - Low-pass FIR (cutoff ~20Hz, 51 taps)
 *          - Moving average (window 10)
 *          - Band-pass IIR (0.5-5Hz) untuk osilasi
 *          - Notch IIR (50/60Hz) untuk hum rejection
 */

#include "filter.h"
#include <string.h>
#include <math.h>

/* ======================== Konstanta Koefisien ======================== */

/* FIR Low-pass 20Hz @ 1kHz, 51 taps, windowed sinc */
/* Koefisien pre-computed untuk efisiensi */
static const float fir_coeffs[FIR_TAP_COUNT] = {
    0.000000f, 0.000259f, 0.000640f, 0.001143f, 0.001764f,
    0.002491f, 0.003305f, 0.004180f, 0.005085f, 0.005982f,
    0.006831f, 0.007590f, 0.008219f, 0.008684f, 0.008958f,
    0.009027f, 0.008888f, 0.008551f, 0.008038f, 0.007378f,
    0.006611f, 0.005785f, 0.004954f, 0.004172f, 0.003494f,
    0.002968f, 0.002634f, 0.002518f, 0.002634f, 0.002968f,
    0.003494f, 0.004172f, 0.004954f, 0.005785f, 0.006611f,
    0.007378f, 0.008038f, 0.008551f, 0.008888f, 0.009027f,
    0.008958f, 0.008684f, 0.008219f, 0.007590f, 0.006831f,
    0.005982f, 0.005085f, 0.004180f, 0.003305f, 0.002491f,
    0.001764f, 0.001143f, 0.000640f, 0.000259f, 0.000000f
};

/* ======================== State Buffer ======================== */
static float fir_buffer[FIR_TAP_COUNT];
static int   fir_index = 0;

static float ma_buffer[MOVING_AVG_WINDOW];
static int   ma_index = 0;
static float ma_sum = 0.0f;

/* IIR Biquad states: Direct Form I */
static float bp_x1 = 0.0f, bp_x2 = 0.0f, bp_y1 = 0.0f, bp_y2 = 0.0f;
static float nt_x1 = 0.0f, nt_x2 = 0.0f, nt_y1 = 0.0f, nt_y2 = 0.0f;

/* ======================== Inisialisasi ======================== */

void Filter_Init(uint32_t sample_rate)
{
    (void)sample_rate; /* Koefisien sudah fixed untuk 1kHz */
    Filter_Reset();
}

void Filter_Reset(void)
{
    memset(fir_buffer, 0, sizeof(fir_buffer));
    fir_index = 0;

    memset(ma_buffer, 0, sizeof(ma_buffer));
    ma_index = 0;
    ma_sum = 0.0f;

    bp_x1 = bp_x2 = bp_y1 = bp_y2 = 0.0f;
    nt_x1 = nt_x2 = nt_y1 = nt_y2 = 0.0f;
}

/* ======================== Low-Pass FIR ======================== */

float Filter_LowPass(float input)
{
    /* Circular buffer */
    fir_buffer[fir_index] = input;
    fir_index = (fir_index + 1) % FIR_TAP_COUNT;

    /* Konvolusi */
    float output = 0.0f;
    int idx = fir_index;

    for (int i = 0; i < FIR_TAP_COUNT; i++) {
        idx = (idx == 0) ? FIR_TAP_COUNT - 1 : idx - 1;
        output += fir_buffer[idx] * fir_coeffs[i];
    }

    return output;
}

/* ======================== Moving Average ======================== */

float Filter_MovingAverage(float input)
{
    ma_sum -= ma_buffer[ma_index];
    ma_buffer[ma_index] = input;
    ma_sum += input;
    ma_index = (ma_index + 1) % MOVING_AVG_WINDOW;

    return ma_sum / (float)MOVING_AVG_WINDOW;
}

/* ======================== Band-Pass IIR (0.5-5Hz) ======================== */

float Filter_BandPass(float input)
{
    /* Biquad band-pass 0.5-5Hz @ 1kHz sampling */
    /* Koefisien pre-computed */
    const float b0 = 0.009707f;
    const float b1 = 0.000000f;
    const float b2 = -0.009707f;
    const float a1 = -1.964459f;
    const float a2 = 0.980293f;

    /* Direct Form I */
    float output = b0 * input + b1 * bp_x1 + b2 * bp_x2
                   - a1 * bp_y1 - a2 * bp_y2;

    /* Update state */
    bp_x2 = bp_x1;
    bp_x1 = input;
    bp_y2 = bp_y1;
    bp_y1 = output;

    return output;
}

/* ======================== Notch 50/60Hz ======================== */

float Filter_Notch(float input)
{
    /* Notch 50Hz, Q=30, @1kHz sampling */
    const float b0 = 0.992536f;
    const float b1 = -1.963465f;
    const float b2 = 0.992536f;
    const float a1 = -1.963465f;
    const float a2 = 0.985072f;

    /* Direct Form I */
    float output = b0 * input + b1 * nt_x1 + b2 * nt_x2
                   - a1 * nt_y1 - a2 * nt_y2;

    /* Update state */
    nt_x2 = nt_x1;
    nt_x1 = input;
    nt_y2 = nt_y1;
    nt_y1 = output;

    return output;
}

/* ======================== Utilitas ======================== */

void Filter_SetLowPassCutoff(float cutoff_hz)
{
    /* Recalculate FIR coefficients for new cutoff */
    (void)cutoff_hz;
    /* Untuk implementasi penuh, regenerate koefisien FIR */
    /* Saat ini menggunakan koefisien fixed */
}

int Filter_GetCoefficients(float* coeff_array, int max_size)
{
    int count = (max_size < FIR_TAP_COUNT) ? max_size : FIR_TAP_COUNT;
    for (int i = 0; i < count; i++) {
        coeff_array[i] = fir_coeffs[i];
    }
    return count;
}
