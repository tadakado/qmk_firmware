/* Copyright 2021 tadakado
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
#include "i2c.h"
#include "rgb_led.h"

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
    LOWER = BMP_SAFE_RANGE,
    RAISE,
    ADJUST,
    CLR,
    STAT,
    DUMMY
};

const key_string_map_t custom_keys_user =
{
    .start_kc = LOWER,
    .end_kc = DUMMY,
    .key_strings = "LOWER\0RAISE\0ADJUST\0CLR\0STAT\0DUMMY\0"
};

// layers, keymaps, keymaps_len will be over-written.

enum layers {
    _BASE, _LOWER, _RAISE, _ADJUST, _NUM, _BLE, _MOUSE
};

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    {{
        // Dummy 
        KC_A, KC_1, KC_H, KC_TAB, KC_SPC,
    }}
};

uint32_t keymaps_len() {
  return sizeof(keymaps)/sizeof(uint16_t);
}

// clear flags for debugging

void clear_modifiers(void) {
    unregister_code(KC_LSFT);
    unregister_code(KC_RSFT);
    unregister_code(KC_LCTL);
    unregister_code(KC_RCTL);
    unregister_code(KC_LGUI);
    unregister_code(KC_RGUI);
    unregister_code(KC_LALT);
    unregister_code(KC_RALT);
}

// Status

void status() {
    uprintf("USB/BLE status: %d %d 0x%04x\r",
            get_usb_enabled(), get_ble_enabled(),
            BMPAPI->ble.get_connection_status());
}

//

static uint16_t ble_stat;

uint16_t ble_connection_status() {
    return BMPAPI->ble.get_connection_status() | (get_ble_enabled()<<15);
}
void keyboard_post_init_kb(void) {
    rgb_init();
    ble_stat = ble_connection_status();
}

void matrix_scan_user(void) {
    uint16_t ble_stat0 = ble_connection_status();
    if (ble_stat0 != ble_stat) {
        ble_stat = ble_stat0;
        rgb_set_connection(ble_stat);
	    rgb_load();
        rgb_update_time();
    }
    rgb_sleep(5);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    rgb_set_layer(state);
    rgb_set_connection(ble_stat);
    rgb_load();
    rgb_update_time();
    return state;
}

void bmp_before_sleep() {
    rgb_set_all(RGB_BLACK);
    rgb_load();
}

// Custom behavior

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool continue_process = process_record_user_bmp(keycode, record);
    if (continue_process == false) {
        return false;
    }

    switch (keycode) {
        case CLR:
            if (record->event.pressed) {
                uprint("##### clear keyboard #####\r");
                layer_clear();
                clear_modifiers();
                clear_keyboard();
            }
            return false;
        case STAT:
            if (record->event.pressed) {
                status();
            }
            return false;
        default:
            break;
    }
    return true;
}
