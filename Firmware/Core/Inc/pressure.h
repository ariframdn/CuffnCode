/**
 * @file    pressure.h
 * @brief   Algoritma pengukuran tekanan darah (Oscillometric Method)
 * @author  Tim CuffnCode
 *
 * @details Implementasi metode oscillometric untuk menentukan:
 *          - Tekanan Sistolik (SBP)
 *          - Tekanan Diastolik (DBP)
 *          - Mean Arterial Pressure (MAP)
 *          - Heart Rate (HR)
 *
 *          Algoritma:
 *          1. Ekstraksi envelope osilasi dari sinyal tekanan
 *          2. MAP = titik puncak envelope osilasi
 *          3. SBP = tekanan saat amplitude = 0.5 * MAP amplitude
 *             (pada sisi kiri / tekanan tinggi)
 *          4. DBP = tekanan saat amplitude = 0.7 * MAP amplitude
 *             (pada sisi kanan / tekanan rendah)
 */

#ifndef __PRESSURE_H
#define __PRESSURE_H

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

/* ======================== Konstanta ======================== */

#define OSCILLOMETRIC_SBP_RATIO     0.55f   /* Rasio SBP/MAP */
#define OSCILLOMETRIC_DBP_RATIO     0.75f   /* Rasio DBP/MAP */
#define MIN_OSCILLATION_AMPLITUDE   0.01f   /* Amplitudo minimum untuk dianggap osilasi */
#define PRESSURE_SAMPLE_WINDOW      500     /* Window untuk perhitungan tekanan (ms) */

/* ======================== Inisialisasi ======================== */

/**
 * @brief  Inisialisasi modul pengukuran tekanan
 */
void Pressure_Init(void);

/* ======================== Pemrosesan ======================== */

/**
 * @brief  Memproses sampel tekanan baru
 * @param  pressure_mmhg: Tekanan cuff dalam mmHg
 * @param  timestamp_ms: Timestamp sampel (ms)
 * @note   Dipanggil setiap ada sampel baru dari ADC
 */
void Pressure_ProcessSample(float pressure_mmhg, uint32_t timestamp_ms);

/**
 * @brief  Mengekstraksi envelope osilasi dari buffer
 * @note   Mencari puncak osilasi dalam window bergerak
 */
void Pressure_ExtractEnvelope(void);

/**
 * @brief  Menghitung SBP, DBP, MAP dari envelope osilasi
 */
void Pressure_CalculateResults(void);

/* ======================== Hasil ======================== */

/**
 * @brief  Mendapatkan hasil pengukuran
 * @return MeasurementResult_t Struktur berisi SBP, DBP, HR, MAP
 */
MeasurementResult_t Pressure_GetResults(void);

/**
 * @brief  Mendapatkan tekanan cuff saat ini
 * @return float Tekanan dalam mmHg
 */
float Pressure_GetCurrentPressure(void);

/**
 * @brief  Mendapatkan sinyal osilasi (untuk debugging/dump)
 * @return float Amplitudo osilasi terkini
 */
float Pressure_GetOscillation(void);

/**
 * @brief  Mengecek apakah pengukuran sedang berlangsung
 * @return true jika sedang mengukur
 */
bool Pressure_IsMeasuring(void);

/* ======================== Manajemen ======================== */

/**
 * @brief  Memulai sesi pengukuran baru
 */
void Pressure_StartMeasurement(void);

/**
 * @brief  Mengakhiri sesi pengukuran
 */
void Pressure_StopMeasurement(void);

/**
 * @brief  Reset semua data pengukuran
 */
void Pressure_Reset(void);

/**
 * @brief  Dump data mentah ke UART untuk debugging/analisis
 */
void Pressure_DumpRawData(void);

#endif /* __PRESSURE_H */
