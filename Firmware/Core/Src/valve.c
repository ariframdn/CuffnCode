/**
 * @file    valve.c
 * @brief   Driver kontrol katup solenoid (deflasi)
 * @author  Tim CuffnCode
 *
 * @details Katup solenoid 3V dikontrol via MOSFET (GPIO PB0).
 *          Membuka saat pengukuran untuk deflasi bertahap ~3-5 mmHg/s.
 *          Quick-dump untuk emergency deflation.
 */

#include "valve.h"
#include "main.h"

/* ======================== Konstanta Deflasi ======================== */

#define DEFLATION_RATE_MMHG_PER_S    4       /* Target deflasi: 4 mmHg/s */
#define DEFLATION_BURST_MS           15      /* Durasi buka katup per burst (ms) */
#define DEFLATION_INTERVAL_MS        100     /* Interval antar burst (ms) */

/* ======================== Variabel Internal ======================== */
static bool valve_open = false;

/* ======================== Inisialisasi ======================== */

void Valve_Init(void)
{
    Valve_Close();
}

/* ======================== Kontrol Dasar ======================== */

void Valve_Open(void)
{
    VALVE_OPEN();
    valve_open = true;
}

void Valve_Close(void)
{
    VALVE_CLOSE();
    valve_open = false;
}

void Valve_SetPosition(uint8_t percentage)
{
    if (percentage == 0) {
        Valve_Close();
    } else if (percentage >= 100) {
        Valve_Open();
    }
    /* Untuk kendali proporsional, bisa pakai PWM via TIM3 */
    /* Implementasi PWM sederhana: duty cycle = percentage */
}

bool Valve_IsOpen(void)
{
    return valve_open;
}

/* ======================== Operasi Deflasi ======================== */

void Valve_QuickDump(void)
{
    Valve_Open();
    HAL_Delay(1000);
    Valve_Close();
}

bool Valve_ProcessDeflation(uint32_t elapsed_ms)
{
    static uint32_t last_burst = 0;
    uint32_t now = HAL_GetTick();

    /* Deflasi: buka katup dalam burst pendek */
    /* Dengan sampling 1kHz dan target 4 mmHg/s */
    /* Setiap burst = buka 15ms, tutup 85ms, setiap 100ms */

    if ((now - last_burst) >= DEFLATION_INTERVAL_MS) {
        Valve_Open();
        HAL_Delay(DEFLATION_BURST_MS);
        Valve_Close();
        last_burst = now;
    }

    /* Deflasi selesai jika tekanan < 20 mmHg atau timeout */
    float pressure = ADC_GetPressure_mmHg();
    if (pressure < 20.0f) {
        Valve_Close();
        return true;
    }

    return false;
}
