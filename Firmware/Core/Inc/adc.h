/**
 * @file    adc.h
 * @brief   Driver ADC untuk pembacaan sensor tekanan
 * @author  Tim CuffnCode
 *
 * @details Mengelola ADC internal STM32F411CE untuk membaca
 *          output dari Analog Front End (AD620 + TLC2272).
 *          Sampling rate: 1 kHz menggunakan trigger timer.
 */

#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>
#include <stdbool.h>

/* ======================== Inisialisasi ======================== */

/**
 * @brief  Inisialisasi ADC dalam mode continuous + timer trigger
 * @note   Menggunakan ADC1, channel 0 (PA0) untuk sensor tekanan
 *         dan channel 1 (PA1) untuk referensi
 */
void ADC_Init(void);

/**
 * @brief  Kalibrasi ADC (offset dan gain)
 */
void ADC_Calibrate(void);

/* ======================== Pembacaan ======================== */

/**
 * @brief  Mendapatkan nilai ADC mentah terakhir
 * @return uint16_t Nilai ADC (0-4095)
 */
uint16_t ADC_GetRawValue(void);

/**
 * @brief  Mendapatkan tegangan terkalibrasi (Volt)
 * @return float Tegangan dalam Volt
 */
float ADC_GetVoltage(void);

/**
 * @brief  Mendapatkan tekanan dalam mmHg
 * @return float Tekanan dalam mmHg
 */
float ADC_GetPressure_mmHg(void);

/**
 * @brief  Memulai sampling ADC dengan DMA + timer trigger
 */
void ADC_StartSampling(void);

/**
 * @brief  Menghentikan sampling ADC
 */
void ADC_StopSampling(void);

/* ======================== DMA Callback ======================== */

/**
 * @brief  Callback ketika setengah buffer DMA terisi
 */
void ADC_HalfCpltCallback(void);

/**
 * @brief  Callback ketika buffer DMA penuh
 */
void ADC_ConvCpltCallback(void);

#endif /* __ADC_H */
