/**
 * @file    filter.h
 * @brief   Filter digital untuk pemrosesan sinyal tekanan
 * @author  Tim CuffnCode
 *
 * @details Implementasi filter digital:
 *          - Low-pass FIR (cutoff ~20 Hz) untuk menghilangkan noise高频
 *          - Moving average untuk smoothing
 *          - Band-pass (0.5-5 Hz) untuk ekstraksi osilasi (oscillometric)
 *          - Notch filter 50/60 Hz untuk menghilangkan hum
 */

#ifndef __FILTER_H
#define __FILTER_H

#include <stdint.h>

/* ======================== Konstanta Filter ======================== */

#define FIR_TAP_COUNT           51      /* Jumlah tap FIR filter */
#define MOVING_AVG_WINDOW       10      /* Window moving average */
#define NOTCH_FREQ_50HZ         50      /* Notch filter 50 Hz */
#define NOTCH_FREQ_60HZ         60      /* Notch filter 60 Hz */

/* ======================== Inisialisasi ======================== */

/**
 * @brief  Inisialisasi semua filter
 * @param  sample_rate: Frekuensi sampling (Hz) untuk kalkulasi koefisien
 */
void Filter_Init(uint32_t sample_rate);

/* ======================== Filter Low-Pass ======================== */

/**
 * @brief  Memproses sampel melalui low-pass FIR filter
 * @param  input: Sampel mentah
 * @return float Sampel yang sudah difilter
 */
float Filter_LowPass(float input);

/* ======================== Moving Average ======================== */

/**
 * @brief  Memproses sampel melalui moving average filter
 * @param  input: Sampel input
 * @return float Rata-rata dari N sampel terakhir
 */
float Filter_MovingAverage(float input);

/* ======================== Band-Pass (Osilasi) ======================== */

/**
 * @brief  Band-pass filter untuk mengekstraksi osilasi tekanan
 * @param  input: Sinyal tekanan DC + osilasi
 * @return float Sinyal osilasi saja (komponen AC)
 */
float Filter_BandPass(float input);

/* ======================== Notch Filter ======================== */

/**
 * @brief  Notch filter untuk menghilangkan hum 50/60 Hz
 * @param  input: Sinyal input
 * @return float Sinyal tanpa komponen hum
 */
float Filter_Notch(float input);

/* ======================== Utilitas ======================== */

/**
 * @brief  Reset state semua filter
 */
void Filter_Reset(void);

/**
 * @brief  Mengatur frekuensi cutoff low-pass filter
 * @param  cutoff_hz: Frekuensi cutoff dalam Hz
 */
void Filter_SetLowPassCutoff(float cutoff_hz);

/**
 * @brief  Mendapatkan koefisien filter untuk debugging
 * @param  coeff_array: Array output untuk koefisien
 * @param  max_size: Ukuran maksimum array
 * @return int Jumlah koefisien
 */
int Filter_GetCoefficients(float* coeff_array, int max_size);

#endif /* __FILTER_H */
