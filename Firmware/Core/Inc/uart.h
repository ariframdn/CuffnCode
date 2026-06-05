/**
 * @file    uart.h
 * @brief   Driver komunikasi serial (UART/USB)
 * @author  Tim CuffnCode
 *
 * @details Menyediakan antarmuka debugging via UART.
 *          Output data real-time: tekanan, osilasi, status.
 *          Format: CSV-friendly untuk logging di PC.
 *          Baudrate: 115200 bps.
 */

#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

/* ======================== Konstanta ======================== */

#define UART_BAUDRATE           115200
#define UART_TX_BUF_SIZE        256
#define UART_RX_BUF_SIZE        64

/* ======================== Inisialisasi ======================== */

/**
 * @brief  Inisialisasi UART untuk debug console
 */
void UART_Init(void);

/* ======================== Transmit ======================== */

/**
 * @brief  Mengirim string melalui UART
 * @param  str: String null-terminated
 */
void UART_SendString(const char* str);

/**
 * @brief  Mengirim data binary melalui UART
 * @param  data: Pointer ke data
 * @param  len: Panjang data dalam byte
 */
void UART_SendData(const uint8_t* data, uint16_t len);

/**
 * @brief  Printf via UART (format string)
 * @param  fmt: Format string (seperti printf)
 * @param  ...: Argumen variadic
 */
void UART_Printf(const char* fmt, ...);

/* ======================== Receive ======================== */

/**
 * @brief  Membaca karakter yang diterima
 * @return char Karakter, atau -1 jika tidak ada data
 */
int16_t UART_ReadChar(void);

/**
 * @brief  Mengecek apakah ada data yang diterima
 * @return uint16_t Jumlah byte dalam RX buffer
 */
uint16_t UART_Available(void);

/* ======================== Output Khusus CuffnCode ======================== */

/**
 * @brief  Mengirim header CSV untuk logging data
 */
void UART_SendCSVHeader(void);

/**
 * @brief  Mengirim satu baris data dalam format CSV
 * @param  timestamp: Timestamp (ms)
 * @param  pressure: Tekanan (mmHg)
 * @param  oscillation: Amplitudo osilasi
 * @param  state: State sistem
 */
void UART_SendCSVData(uint32_t timestamp, float pressure,
                      float oscillation, uint8_t state);

/**
 * @brief  Mengirim hasil pengukuran dalam format JSON
 * @param  result: Hasil pengukuran
 */
void UART_SendJSONResult(MeasurementResult_t result);

#endif /* __UART_H */
