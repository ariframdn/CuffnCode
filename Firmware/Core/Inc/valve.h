/**
 * @file    valve.h
 * @brief   Driver kontrol katup solenoid untuk deflasi cuff
 * @author  Tim CuffnCode
 *
 * @details Mengontrol katup solenoid melalui MOSFET (GPIO).
 *          Katup membuka untuk deflasi bertahap selama pengukuran.
 *          Kecepatan deflasi: ~3-5 mmHg/detik sesuai standar medis.
 */

#ifndef __VALVE_H
#define __VALVE_H

#include <stdint.h>
#include <stdbool.h>

/* ======================== Inisialisasi ======================== */

/**
 * @brief  Inisialisasi pin GPIO untuk kontrol katup
 */
void Valve_Init(void);

/* ======================== Kontrol ======================== */

/**
 * @brief  Membuka katup (deflasi cuff)
 */
void Valve_Open(void);

/**
 * @brief  Menutup katup
 */
void Valve_Close(void);

/**
 * @brief  Mengatur bukaan katup (PWM)
 * @param  percentage: Persentase bukaan (0-100)
 * @note   Untuk kontrol deflasi yang presisi
 */
void Valve_SetPosition(uint8_t percentage);

/**
 * @brief  Mendapatkan status katup
 * @return true jika katup terbuka
 */
bool Valve_IsOpen(void);

/* ======================== Operasi ======================== */

/**
 * @brief  Membuka katup sepenuhnya (deflasi cepat / emergency)
 */
void Valve_QuickDump(void);

/**
 * @brief  Proses deflasi terkontrol (dipanggil dari state machine)
 * @param  elapsed_ms: Waktu sejak deflasi dimulai (ms)
 * @return true jika deflasi selesai
 */
bool Valve_ProcessDeflation(uint32_t elapsed_ms);

#endif /* __VALVE_H */
