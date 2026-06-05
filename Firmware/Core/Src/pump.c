/**
 * @file    pump.c
 * @brief   Driver kontrol pompa cuff (inflasi)
 * @author  Tim CuffnCode
 *
 * @details Pompa mini dikontrol via MOSFET (GPIO PA8).
 *          Inflasi bertahap hingga tekanan target ~180mmHg.
 *          Proteksi over-pressure dan timeout.
 */

#include "pump.h"
#include "main.h"

/* ======================== Variabel Internal ======================== */
static bool pump_active   = false;
static uint16_t pump_target = CUFF_TARGET_PRESSURE;
static uint32_t pump_start_time = 0;

/* ======================== Inisialisasi ======================== */

void Pump_Init(void)
{
    pump_active = false;
    pump_target = CUFF_TARGET_PRESSURE;
    Pump_Off();
}

/* ======================== Kontrol Dasar ======================== */

void Pump_On(void)
{
    PUMP_ON();
    pump_active = true;
}

void Pump_Off(void)
{
    PUMP_OFF();
    pump_active = false;
}

bool Pump_IsOn(void)
{
    return pump_active;
}

/* ======================== Inflasi ======================== */

bool Pump_InflateTo(uint16_t target_pressure)
{
    pump_target = target_pressure;
    pump_start_time = HAL_GetTick();

    Pump_On();

    /* Loop blocking - hanya untuk debugging */
    uint32_t timeout_ms = MAX_PUMP_TIME_MS;
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms) {
        float current = ADC_GetPressure_mmHg();

        if (current >= (float)target_pressure) {
            Pump_Off();
            return true;
        }

        HAL_Delay(10);
    }

    Pump_Off();
    return false; /* Timeout */
}

bool Pump_Process(uint16_t current_pressure)
{
    static uint32_t last_on_time = 0;
    uint32_t now = HAL_GetTick();

    /* Inisialisasi start time saat pertama dipanggil */
    if (!pump_active) {
        Pump_On();
        pump_start_time = now;
        last_on_time = now;
        return false;
    }

    /* Cek apakah target tercapai */
    if (current_pressure >= pump_target) {
        Pump_Off();
        return true;
    }

    /* Proteksi over-pressure */
    if (current_pressure > pump_target + 20) {
        Pump_Off();
        return true;
    }

    /* Kontrol proporsional: kurangi kecepatan saat mendekati target */
    uint16_t remaining = (current_pressure >= pump_target) ? 0 :
                          pump_target - current_pressure;

    if (remaining < 20) {
        /* PWM: pulse pendek saat mendekati target */
        if ((now - last_on_time) > 50) {
            Pump_On();
            HAL_Delay(20);
            Pump_Off();
            last_on_time = now;
        }
    }

    return false;
}

void Pump_Reset(void)
{
    pump_active = false;
    pump_start_time = 0;
    Pump_Off();
}
