/* Copyright 2019 sekigon-gonnoc, 2022 tadakado
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "bmp.h"
#include "bmp_custom_keycode.h"
#include "keycode_str_converter.h"
#include "bmp_indicator_led.h"

// Modify config
bool bmp_config_overwrite(bmp_api_config_t const *const config_on_storage,
                          bmp_api_config_t *const       keyboard_config) {
  keyboard_config->startup = 1;
  return true;
}

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
    LOWER = BMP_SAFE_RANGE,
    MUSIC,
    PRSTT
};

const key_string_map_t custom_keys_user =
{
    .start_kc = LOWER,
    .end_kc = PRSTT,
    .key_strings = "LOWER\0MUSIC\0PRSTT\0"
};

enum layers {
    _MUSIC, _PRSTT, _BLE, _CONF, 
};

// Tap Dance Declarations
enum {
    TD_MNXT_MPRV = 0
};

// Tap Dance Definitions (put "TAP_DANCE_ENABLE = yes" in rules.mk)
qk_tap_dance_action_t tap_dance_actions[] = {
    [TD_MNXT_MPRV] = ACTION_TAP_DANCE_DOUBLE(KC_MNXT, KC_MPRV)
};

// MATRIX_ROWS and MATRIX_COLS are 32 
const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {{{
  LOWER, KC_MPLY, KC_VOLD, KC_VOLU, TD(TD_MNXT_MPRV),
  LOWER, KC_ESC,  KC_LEFT, KC_RGHT, KC_F5,
  KC_NO, ADV_ID0, ADV_ID1, ADV_ID2, ADV_ID3,
  KC_NO, AD_WO_L, MUSIC,   PRSTT,   DELBNDS,
}}};

uint32_t keymaps_len() {
  return 20;
}

static bool lower_pressed = false;
static uint16_t lower_pressed_time = 0;

// LED indicator (see tmk_core/protocol/nrf/bmp_indicator_led.c)
int bmp_indicator_user_pattern(uint32_t time_ms, int32_t option) {
/*
  if (time_ms > 3000 && option < 8) {
    bmp_indicator_led_off();
    return 1;
  }
*/
  uint32_t time_ms_mod = time_ms % 200;
  switch(option) {
    case 0:
      if (time_ms_mod < 50) {
        bmp_indicator_led_on();
      } else {
        bmp_indicator_led_off();
        return 1;
      }
      break;
    case 1:
      if (time_ms_mod < 50) {
        bmp_indicator_led_on();
      } else {
        bmp_indicator_led_off();
        if (time_ms > 250) return 1;
      }
      break;
    case 2:
      if (time_ms_mod < 50) {
        bmp_indicator_led_on();
      } else {
        bmp_indicator_led_off();
        if (time_ms > 450) return 1;
      }
      break;
    case 3:
      if (time_ms_mod < 50) {
         bmp_indicator_led_on();
      } else {
        bmp_indicator_led_off();
        if (time_ms > 650) return 1;
      }
      break;
    case 8:
      time_ms_mod = time_ms % 400;
      if (time_ms_mod < 300) {
         bmp_indicator_led_on();
      } else {
        bmp_indicator_led_off();
        if (IS_LAYER_OFF(_CONF)) return 1;
      }
      break;
    default:
      bmp_indicator_led_off();
      break;
  }
  return 0;
}

// BLE status report on indicator LED (see tmk_core/protocol/nrf/sdk15/cli.c)
void report_ble_status_on_led() {
  // stat >> 8 : number of BLE connections (0 or 1)
  // stat & 0xff : BLE connection id (0 to 7)
  uint16_t stat = BMPAPI->ble.get_connection_status();
  if (stat >> 8) {
    bmp_indicator_set(INDICATOR_USER, stat & 0xff);
  }
}

void indicate_setting_mode_on_led() {
  bmp_indicator_set(INDICATOR_USER, 8);
}

void matrix_scan_user(void) {
  // When LOWER is pressed > 5 sec. it goes to RAISE layer
  if (lower_pressed) {
    if (TIMER_DIFF_16(timer_read()|1, lower_pressed_time) > 5000) {
      if (IS_LAYER_OFF(_CONF)) {
        indicate_setting_mode_on_led();
        layer_off(_BLE);
        layer_on(_CONF);
      }
    }
  }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  bool continue_process = process_record_user_bmp(keycode, record);
  if (continue_process == false)
  {
    return false;
  }

  switch (keycode) {
    case LOWER:
      if (record->event.pressed) {
        lower_pressed = true;
        lower_pressed_time = record->event.time;
        layer_on(_BLE);
      } else {
        layer_off(_BLE);
        layer_off(_CONF); // LOWER > 5 sec.
        if (lower_pressed) {
          lower_pressed = false;
          // Tapping
          if (TIMER_DIFF_16(record->event.time, lower_pressed_time) < TAPPING_TERM) {
            report_ble_status_on_led();
          }
        }
      }
      return false;
      break;
    case MUSIC:
      if (record->event.pressed) {
        print("MUSIC mode\r");
        default_layer_set((layer_state_t)1 << _MUSIC);
      }
      return false;
      break;
    case PRSTT:
      if (record->event.pressed) {
        print("PRSTT mode\r");
        default_layer_set((layer_state_t)1 << _PRSTT);
      }
      return false;
      break;
    default:
      break;
  }

  return true;
}
