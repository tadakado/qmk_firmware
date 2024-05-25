/*
 * rgb_led.h
 *
 *  Created on: 2024/3/24
 *      Author: tadakado
 */

#pragma once

#define RGB_LEFT 1
#define RGB_RIGHT 2
#define RGB_BOTH (RGB_LEFT | RGB_RIGHT)

extern uint8_t colors[][3];

void rgb_load(void);
void rgb_set(uint8_t p, uint8_t i, uint8_t r, uint8_t g, uint8_t b);
void rgb_set_all(uint8_t r, uint8_t g, uint8_t b);
void rgb_init(void);
void rgb_set_layer(layer_state_t state);
void rgb_set_connection(uint16_t ble_stat);
void rgb_sleep(uint8_t duration);
void rgb_update_time(void);