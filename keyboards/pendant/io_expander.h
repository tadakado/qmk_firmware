/*
 * io_expander.h
 *
 *  Created on: 2018/12/21
 *      Author: sekigon-gonnoc
 *      Modifyed by tadakado (2023/10)
 */

#pragma once

#include "matrix.h"

typedef struct {
    uint8_t addr;
    uint8_t row_pins[32];
    uint8_t row_num;
    uint8_t col_pins[32];
    uint8_t col_num;
    uint8_t io_expander_init_packet[6];
    bool reconfig_flag;
    bool row_prepare_flag;
} io_expander_config_t;

void     io_expander_init(io_expander_config_t *const config);
uint32_t io_expander_scan(io_expander_config_t *const config, matrix_row_t *rows);
