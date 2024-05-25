/*
 * rgb_led.c
 *
 *  Created on: 2024/3/24
 *      Author: tadakado
 */

#include "bmp.h"
#include "i2c.h"
#include "rgblight.h"
#include "timer.h"
#include "action_layer.h"

#define N_CON 2
#define RGB_LEFT 1
#define RGB_RIGHT 2
#define RGB_BOTH (RGB_LEFT | RGB_RIGHT)

uint32_t rgb_off_time;

// ==================================================
// RGB LED

uint8_t colors[][3] =
	{{RGB_RED}, {RGB_GREEN}, {RGB_BLUE}, {RGB_YELLOW}, {RGB_MAGENTA},
	 {RGB_CYAN}, {RGB_WHITE}, {RGB_ORANGE}, {RGB_PINK}, {RGB_PURPLE}};
static uint8_t rgb_addr[N_CON] = {I2C_7BIT_ADDR(0x31), I2C_7BIT_ADDR(0x30)};
static uint8_t rgb_load_command[] = {0xff, N_RGB};
struct {
    uint8_t addr;
    RGB rgb[N_RGB];
} static rgb_data[N_CON];

void rgb_load(void) {
  i2c_init();
  for (uint8_t s=0; s<N_CON; s++) {
    i2c_transmit(rgb_addr[s], (uint8_t *)&rgb_data[s], sizeof(rgb_data[s]), 0);
    i2c_transmit(rgb_addr[s], rgb_load_command, sizeof(rgb_load_command), 0);
  }
  i2c_uninit();
}

void rgb_set(uint8_t p, uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t s=0; s<N_CON; s++) {
    if (p & 1<<s) {
      setrgb(r, g, b, (LED_TYPE *)&rgb_data[s].rgb[i]);
    }
  }
}

void rgb_set_all(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t s=0; s<N_CON; s++) {
    for (uint8_t i=0; i<N_RGB; i++) {
      setrgb(r, g, b, (LED_TYPE *)&rgb_data[s].rgb[i]);
    }
  }
}

void rgb_update_time() {
  rgb_off_time = timer_read32();
}

void rgb_init(void) {
  for (uint8_t i=0; i<sizeof(colors)/sizeof(colors[0]); i++) {
    colors[i][0] >>= 1; // red
    colors[i][1] >>= 1; // green
    colors[i][2] >>= 1; // blue
  }
  for (uint8_t s=0; s<N_CON; s++) rgb_data[s].addr = 0x00;
  rgb_set_all(RGB_RED);
  rgb_load();
  rgb_update_time();
}

void rgb_set_layer(layer_state_t state) {
  uint8_t l = get_highest_layer(state);
  rgb_set_all(colors[l][0], colors[l][1], colors[l][2]);
}

void rgb_set_connection(uint16_t ble_stat) {
  // ble_stat >> 15 : BLE connection flag (0 or 1)
  // ble_stat & 0xff : BLE connection id (0 to 7) or nobonding (0xff)
  uint8_t l;
  if (ble_stat & 1<<15) {
    l = (ble_stat & 0xff) + 1;
  } else {
    l = 0;
  }
  rgb_set(RGB_BOTH, STATUS_RGB, colors[l][0], colors[l][1], colors[l][2]);
}

void rgb_sleep(uint8_t duration) {
  if (timer_elapsed32(rgb_off_time) / 1000 >= duration) {
    rgb_set_all(RGB_BLACK);
    rgb_load();
  }
}
