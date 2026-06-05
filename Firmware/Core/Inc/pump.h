/**
 * @file    pump.h
 * @brief   Driver kontrol pompa udara untuk cuff
 * @author  Tim CuffnCode
 *
 * @details Mengontrol pompa mini melalui MOSFET (GPIO).
 *          Pompa akan mengembang cuff hingga tekanan target.
 *          Dilengkapi proteksi over-pressure dan timeout.
 */

#ifndef __PUMP_H
#define __PUMP_H

#include <stdint.h>
#include <stdbool.h>

/* ======================== Inisialisasi ======================== */

/**
 * @brief  Inisialisasi pin GPIO untuk kontrol pompa
 */
void Pump_Init(void);

/* ======================== Kontrol ======================== */

/**
 * @brief  Mengaktifkan pompa (cuff mengembang)
 */
void Pump_On(void);

/**
 * @brief  Mematikan pompa
 */
void Pump_Off(void);

/**
 * @brief  Mendapatkan status pompa
 * @return true jika pompa aktif
 */
bool Pump_IsOn(void);

/**
 * @brief  Memompa hingga tekanan target tercapai
 * @param  target_pressure: Tekanan target dalam mmHg
 * @return true jika berhasil mencapai target
 * @return false jika timeout atau error
 */
bool Pump_InflateTo(uint16_t target_pressure);

/* ======================== Operasi ======================== */

/**
 * @brief  Proses inflasi (dipanggil dari state machine)
 * @param  current_pressure: Tekanan saat ini (mmHg)
 * @return true jika target tercapai, false jika masih pumping
 */
bool Pump_Process(uint16_t current_pressure);

/**
 * @brief  Reset state pompa
 */
void Pump_Reset(void);

#endif /* __PUMP_H */
