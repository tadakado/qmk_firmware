/*
 * io_expander.c
 *
 *  Created on: 2018/12/16
 *      Author: sekigon-gonnoc
 *      Modifyed by tadakado (2023/10)
 */

#include <stdint.h>
#include "io_expander.h"
#include "quantum.h"
#include "apidef.h"
#include "i2c.h"
#include "spi.h"
#include "matrix.h"
#include "apidef.h"

static const uint8_t io_expander_pin_def[24] = {
    0, // pin number in config starts from 1
    0, 1, 0, 0, 0, 0,
    2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13,
    14, 15, 0, 0, 0
};
#define IO_EXPANDER_REG_INPUT0     0x00
#define IO_EXPANDER_REG_OUTPUT0    0x02
#define IO_EXPANDER_REG_CONFIG0    0x06

void io_expander_init(io_expander_config_t * const config)
{
    uint8_t pin;

    config->io_expander_init_packet[0] = 0xFF; // reg 2: Output port register
    config->io_expander_init_packet[1] = 0xFF;
    config->io_expander_init_packet[2] = 0x00; // reg 4: Polarity inversion register
    config->io_expander_init_packet[3] = 0x00;
    config->io_expander_init_packet[4] = 0xFF; // reg 6: Configuration register
    config->io_expander_init_packet[5] = 0xFF;

    // set row pins as output
    for (int row=0; row<config->row_num; row++)
    {
        pin = io_expander_pin_def[config->row_pins[row]];
        if (pin < 8)
        {
            config->io_expander_init_packet[0] &= ~(1<<pin);
            config->io_expander_init_packet[4] &= ~(1<<pin);
        }
        else
        {
            config->io_expander_init_packet[1] &= ~(1 << (pin-8));
            config->io_expander_init_packet[5] &= ~(1 << (pin-8));
        }
    }
}

static matrix_row_t io_expander_read_col_on_row(io_expander_config_t const * const config)
{
    uint16_t col;

    // read all col pins
    if (i2c_readReg(config->addr, IO_EXPANDER_REG_INPUT0,
        (uint8_t*)&col, sizeof(col), 0))
    {
        return 0;
    }

    matrix_row_t retval = 0;

    for (int col_idx=0; col_idx<config->col_num; col_idx++)
    {
        uint8_t pin = io_expander_pin_def[config->col_pins[col_idx]];
        retval |= (col & (1 << pin) ?  0 : (1 << col_idx));
    }

    return retval;
}

static bool io_expander_first_scan(io_expander_config_t * const config)
{
    if (config->row_prepare_flag == false)
    {
        // set all row pins to low
        i2c_writeReg(config->addr, IO_EXPANDER_REG_OUTPUT0,
            config->io_expander_init_packet, 2, 0);
        config->row_prepare_flag = true;
    }

    if (config->reconfig_flag)
    {
        // reset all pins I/O setting
        i2c_writeReg(config->addr, IO_EXPANDER_REG_CONFIG0,
            &config->io_expander_init_packet[4], 2, 0);
        config->reconfig_flag = false;
    }

    uint16_t col;
 
    // read all pin state
    i2c_readReg(config->addr, IO_EXPANDER_REG_INPUT0,
        (uint8_t*)&col, sizeof(col), 0);

    for (int row_idx=0; row_idx<config->row_num; row_idx++)
    {
        uint8_t pin = io_expander_pin_def[config->row_pins[row_idx]];
        if ((col & (1 << pin)) != 0)
        {
            // if row pin is high (though it should be set to low),
            // reconfig is required
            config->reconfig_flag = true;
            break;
        }
    }

    for (int col_idx=0; col_idx<config->col_num; col_idx++)
    {
        uint8_t pin = io_expander_pin_def[config->col_pins[col_idx]];
        if ((col & (1 << pin)) == 0)
        {
            // some switch is pressed
            return true;
        }
    }

    return false;
}

static matrix_row_t io_expander_read_row(io_expander_config_t const * const config, uint8_t row)
{
    uint8_t send_dat[2] = {0xFF, 0xFF};

    uint8_t pin = io_expander_pin_def[config->row_pins[row]];
    if (pin < 8)
    {
        send_dat[0] &= ~(1<<pin);
    }
    else
    {
        send_dat[1] &= ~(1<<(pin-8));
    }

    // set a row pin to low
    i2c_writeReg(config->addr, IO_EXPANDER_REG_OUTPUT0,
        send_dat, sizeof(send_dat), 0);

    return io_expander_read_col_on_row(config);
}

extern const uint8_t MAINTASK_INTERVAL;
uint32_t io_expander_scan(io_expander_config_t *const config, matrix_row_t *rows)
{
    uint32_t        change                 = 0;

    i2c_init();
    matrix_row_t row;
    if (io_expander_first_scan(config)) {
        // some switches are pressed
        for (int row_idx = 0; row_idx < config->row_num; row_idx++) {
            row = io_expander_read_row(config, row_idx);
            if (rows[row_idx] ^ row) {
                rows[row_idx] = row;
                change        = 1;
            }
        }
        config->row_prepare_flag = false;
    } else {
        // no switch is pressed
        for (int row_idx = 0; row_idx < config->row_num; row_idx++) {
            if (rows[row_idx]) {
                rows[row_idx] = 0;
                change        = 1;
            }
        }
    }

    i2c_uninit();

    return change;
}
