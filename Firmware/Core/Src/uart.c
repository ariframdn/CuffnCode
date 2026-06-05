/**
 * @file    uart.c
 * @brief   Driver komunikasi serial UART untuk debugging
 * @author  Tim CuffnCode
 *
 * @details USART2 @115200 baud, 8N1.
 *          Output format: CSV untuk data logging, JSON untuk hasil.
 *          RX interrupt untuk command processing.
 */

#include "uart.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ======================== Ring Buffer ======================== */

typedef struct {
    uint8_t buffer[UART_TX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
} RingBuffer_t;

static RingBuffer_t tx_ring;
static RingBuffer_t rx_ring;

/* TX completion flag */
static volatile uint8_t tx_busy = 0;
static uint8_t tx_data[UART_TX_BUF_SIZE];

/* ======================== Inisialisasi ======================== */

void UART_Init(void)
{
    memset(&tx_ring, 0, sizeof(tx_ring));
    memset(&rx_ring, 0, sizeof(rx_ring));
    tx_busy = 0;
}

/* ======================== Ring Buffer Operations ======================== */

static int RingBuffer_Put(RingBuffer_t* rb, uint8_t data)
{
    if (rb->count >= UART_TX_BUF_SIZE) return -1; /* Full */
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % UART_TX_BUF_SIZE;
    rb->count++;
    return 0;
}

static int RingBuffer_Get(RingBuffer_t* rb, uint8_t* data)
{
    if (rb->count == 0) return -1; /* Empty */
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % UART_TX_BUF_SIZE;
    rb->count--;
    return 0;
}

/* ======================== Transmit ======================== */

void UART_SendString(const char* str)
{
    while (*str) {
        if (RingBuffer_Put(&tx_ring, (uint8_t)*str) != 0) break;
        str++;
    }

    /* Start TX jika tidak sedang busy */
    if (!tx_busy) {
        tx_busy = 1;
        uint16_t len = 0;
        while (RingBuffer_Get(&tx_ring, &tx_data[len]) == 0 && len < UART_TX_BUF_SIZE) {
            len++;
        }
        if (len > 0) {
            HAL_UART_Transmit_IT(&huart2, tx_data, len);
        } else {
            tx_busy = 0;
        }
    }
}

void UART_SendData(const uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        RingBuffer_Put(&tx_ring, data[i]);
    }

    if (!tx_busy) {
        tx_busy = 1;
        uint16_t send_len = 0;
        while (RingBuffer_Get(&tx_ring, &tx_data[send_len]) == 0 && send_len < UART_TX_BUF_SIZE) {
            send_len++;
        }
        if (send_len > 0) {
            HAL_UART_Transmit_IT(&huart2, tx_data, send_len);
        } else {
            tx_busy = 0;
        }
    }
}

void UART_Printf(const char* fmt, ...)
{
    char buffer[UART_TX_BUF_SIZE];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0) {
        UART_SendString(buffer);
    }
}

/* ======================== TX Complete Callback ======================== */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        /* Kirim data berikutnya dari ring buffer */
        uint16_t len = 0;
        while (RingBuffer_Get(&tx_ring, &tx_data[len]) == 0 && len < UART_TX_BUF_SIZE) {
            len++;
        }
        if (len > 0) {
            HAL_UART_Transmit_IT(&huart2, tx_data, len);
        } else {
            tx_busy = 0;
        }
    }
}

/* ======================== Receive ======================== */

int16_t UART_ReadChar(void)
{
    uint8_t data;
    if (RingBuffer_Get(&rx_ring, &data) == 0) {
        return (int16_t)data;
    }
    return -1;
}

uint16_t UART_Available(void)
{
    return rx_ring.count;
}

/* RX callback - dipanggil dari interrupt */
void UART_RX_Char(uint8_t data)
{
    RingBuffer_Put(&rx_ring, data);
}

/* ======================== Output Khusus ======================== */

void UART_SendCSVHeader(void)
{
    UART_Printf("timestamp_ms,pressure_mmHg,oscillation,state\r\n");
}

void UART_SendCSVData(uint32_t timestamp, float pressure,
                      float oscillation, uint8_t state)
{
    UART_Printf("%lu,%.2f,%.4f,%u\r\n",
                timestamp, pressure, oscillation, state);
}

void UART_SendJSONResult(MeasurementResult_t result)
{
    UART_Printf("{\r\n");
    UART_Printf("  \"systolic\": %u,\r\n", result.systolic);
    UART_Printf("  \"diastolic\": %u,\r\n", result.diastolic);
    UART_Printf("  \"map\": %.1f,\r\n", result.map);
    UART_Printf("  \"heart_rate\": %u,\r\n", result.heart_rate);
    UART_Printf("  \"valid\": %s\r\n", result.valid ? "true" : "false");
    UART_Printf("}\r\n");
}
