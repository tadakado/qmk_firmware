/*
 * uart_ch559.c
 *
 *  Created on: 2024/3/24
 *      Author: tadakado
 */

#include <string.h>
#include "apidef.h"
#include "report.h"
#include "pointing_device.h"
#include <print.h>
#include "debug.h"

#define BUF_LEN 256 /// DO NOT CHANGE!! This code relies on uint8_t's wrap-around behavior at 256 (mod 256) when underflow occurs.
#define BUF_IDX_MASK (BUF_LEN - 1)

static int8_t buf[BUF_LEN];
static uint8_t start;
static uint8_t end;

static report_mouse_t mouse_report;
static uint8_t prev_buttons = 0;
static bool prev_updated = false;

static int8_t buttons = 0;
static int16_t x = 0, y = 0;
static int8_t v = 0, h = 0;

#define CH559_HEADER (int8_t)0x80 // Match the same type with int8_t buf[]
#define CH559_HEADER_OFFSET 1
#define CH559_DATA_LEN 6
#define CH559_BUTTONS (CH559_HEADER_OFFSET+0)
#define CH559_XX (CH559_HEADER_OFFSET+1)
#define CH559_YY (CH559_HEADER_OFFSET+2)
#define CH559_VV (CH559_HEADER_OFFSET+3)
#define CH559_HH (CH559_HEADER_OFFSET+4)

static inline int8_t clamp(int16_t value) {
    if (value < -127) return -127;
    if (value > 127) return 127;
    return value;
}

int16_t find_start_pos(){
    uint8_t i;
    uint8_t data_len;

    data_len = end - start;
    if (data_len < CH559_DATA_LEN) return -1;
    for (i=0; i<=data_len-CH559_DATA_LEN; i++) {
        if (buf[start + i] == CH559_HEADER) {
            return i;
        }
    }
    return -1;
}

void ch559_parse(void) {
    start += CH559_HEADER_OFFSET;
    buttons |= (uint8_t)buf[start++]; // CH559_BUTTONS
    x += buf[start++]; // CH559_XX
    y += buf[start++]; // CH559_YY
    v += buf[start++]; // CH559_VV
    h += buf[start++]; // CH559_HH
}

bool uart_ch559_parser(void) {
    uint8_t i;
    int16_t idx;
    bool updated = false;

    buttons = 0;
    x = 0;
    y = 0;
    v = 0;
    h = 0;

    for(i=0; i<8; i++) {
        idx = find_start_pos();
        if (idx < 0) break;
        if (true && debug_mouse){
            for (int j=0; j<CH559_DATA_LEN; j++) dprintf("%02x ", (uint8_t)buf[start + idx + j]);
            dprintf(": %02d %03d %03d\r", idx, start, end);
        }
        start += (uint8_t)idx;
        ch559_parse();
        updated = true;
    }
    if (updated) mouse_report.buttons = buttons;
    else mouse_report.buttons = prev_buttons;
    mouse_report.x = clamp(x);
    mouse_report.y = clamp(y);
    mouse_report.v = clamp(v);
    mouse_report.h = clamp(h);
    if (false && debug_mouse)
        dprintf("%d %d %02x %d %d %d %d: %d %d\r", updated, i,
            mouse_report.buttons, mouse_report.x, mouse_report.y, mouse_report.v, mouse_report.h,
            start, end);
    return updated;
}

void uart_ch559_report() {
    bool updated;

    BMPAPI->uart.send(NULL, 0);
    updated = uart_ch559_parser();
    if (updated || prev_updated) {
        pointing_device_set_shared_report(mouse_report);
        prev_buttons = mouse_report.buttons;
        prev_updated = updated;
    }
}

void uart_recv_callback(uint8_t dat) {
    buf[end++] = dat;
}

void uart_ch559_init(void) {
    memset(buf, 0, sizeof(buf));
    start = 0;
    end = 0;

    bmp_uart_config_t uart_config;
    uart_config.tx_pin      = 1;
    uart_config.rx_pin      = 20;
    uart_config.rx_callback = uart_recv_callback;
    uart_config.baudrate    = 57600;
    uart_config.rx_protocol = 0;
    BMPAPI->uart.init(&uart_config);

    mouse_report.buttons = 0;
    mouse_report.x = 0;
    mouse_report.y = 0;
    mouse_report.v = 0;
    mouse_report.h = 0;
}
