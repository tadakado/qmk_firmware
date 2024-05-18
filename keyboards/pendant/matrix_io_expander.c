
#include "io_expander.h"
#include "bmp_matrix.h"
#include "quantum.h"
#include "apidef.h"
#include "i2c.h"

static void init_col2row();
static uint32_t get_device_row();
static uint32_t get_device_col();
static uint32_t scan_col2row();

static void init_col2row_dummy();
static uint32_t scan_col2row_dummy();

const bmp_matrix_func_t matrix_func_col2row_io_expander = {init_col2row, get_device_row, get_device_col, scan_col2row};
const bmp_matrix_func_t matrix_func_col2row_io_expander_dummy = {init_col2row_dummy, get_device_row, get_device_col, scan_col2row_dummy};

static uint8_t io_expander_row_num, io_expander_col_num;
static uint8_t pin_matrix_row_num, pin_matrix_col_num;
static io_expander_config_t io_expander_config[2];

static uint32_t get_device_row()
{
    return io_expander_row_num + pin_matrix_row_num;
}

static uint32_t get_device_col()
{
    return pin_matrix_col_num;
}

static uint32_t get_io_expander_row_num()
{
    const bmp_api_config_t *config = BMPAPI->app.get_config();
    uint32_t row_pin_idx;

    for (row_pin_idx=0; row_pin_idx<sizeof(config->matrix.row_pins); row_pin_idx++) {
        if (config->matrix.row_pins[row_pin_idx] == 0)
        {
            break;
        }
    }

    if (row_pin_idx < config->matrix.device_rows)
    {
        return 0;
    }
    else
    {
        return row_pin_idx - config->matrix.device_rows;
    }
}

static uint32_t get_io_expander_col_num()
{
    const bmp_api_config_t *config = BMPAPI->app.get_config();
    uint32_t col_pin_idx;

    for (col_pin_idx=0; col_pin_idx<sizeof(config->matrix.col_pins); col_pin_idx++) {
        if (config->matrix.col_pins[col_pin_idx] == 0)
        {
            break;
        }
    }

    if (col_pin_idx < config->matrix.device_cols)
    {
        return 0;
    }
    else
    {
        return col_pin_idx - config->matrix.device_cols;
    }
}

//
//// col2row matrix
//
static void init_col2row() {
    const bmp_api_config_t *config = BMPAPI->app.get_config();

    pin_matrix_row_num = config->matrix.device_rows;
    pin_matrix_col_num = config->matrix.device_cols;
    io_expander_row_num = get_io_expander_row_num();
    io_expander_col_num = get_io_expander_col_num();

    io_expander_config[0].addr = I2C_7BIT_ADDR(0x21);
    io_expander_config[0].row_num = pin_matrix_row_num;
    io_expander_config[0].col_num = pin_matrix_col_num;
    io_expander_config[0].reconfig_flag = true;
    io_expander_config[0].row_prepare_flag = false;

    for (int r=0; r<pin_matrix_row_num; r++) {
        io_expander_config[0].row_pins[r] = config->matrix.row_pins[r];
    }

    for (int c=0; c<pin_matrix_col_num; c++) {
        io_expander_config[0].col_pins[c] = config->matrix.col_pins[c];
    }

    io_expander_init(&io_expander_config[0]);

    io_expander_config[1].addr = I2C_7BIT_ADDR(0x20);
    io_expander_config[1].row_num = io_expander_row_num;
    io_expander_config[1].col_num = io_expander_col_num;
    io_expander_config[1].reconfig_flag = true;
    io_expander_config[1].row_prepare_flag = false;

    for (int r=0; r<io_expander_row_num; r++) {
        io_expander_config[1].row_pins[r] = config->matrix.row_pins[r + pin_matrix_row_num];
    }

    for (int c=0; c<io_expander_col_num; c++) {
        io_expander_config[1].col_pins[c] = config->matrix.col_pins[c + pin_matrix_col_num];
    }

    io_expander_init(&io_expander_config[1]);
}


static uint32_t scan_col2row(matrix_row_t *matrix_raw)
{
    const bmp_api_config_t *config = BMPAPI->app.get_config();
    uint32_t change = 0;

    uint8_t matrix_offset_slave = 0;
    change |= io_expander_scan(&io_expander_config[0], &matrix_raw[matrix_offset_slave]);

    matrix_offset_slave = config->matrix.device_rows;
    change |= io_expander_scan(&io_expander_config[1], &matrix_raw[matrix_offset_slave]);

    return change;
}

static void init_col2row_dummy()
{
}
 
static uint32_t scan_col2row_dummy(matrix_row_t *matrix_raw)
{
    return (uint32_t)0;
}
