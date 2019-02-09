#include <string.h>
#include "ftdi_util.h"

int ftdi_set_10mhz_clk(struct ftdi_context *ftdi)
{
    int cmd_len = 3;
    u_int8_t cmd_bytes[cmd_len];
    
    cmd_bytes[0] = DIS_DIV_5;       // use 60 MHz clock, not 12 MHz
    cmd_bytes[1] = DIS_ADAPTIVE;    // disable adaptive clocking
    cmd_bytes[2] = DIS_3_PHASE;     // disable 3-phase clocking
    check_error(ftdi_write_data(ftdi, cmd_bytes, cmd_len));
    
    // Set CLK = 60 / ((1 + 2) *2) = 10 MHz
    cmd_bytes[0] = TCK_DIVISOR; // set divisor:
    cmd_bytes[1] = 2; // low byte 2
    cmd_bytes[2] = 0; // high byte 0
    check_error(ftdi_write_data(ftdi, cmd_bytes, cmd_len));
    return 0;
}

int ftdi_send_bytes(struct ftdi_context *ftdi, uint8_t *data, uint16_t len)
{
    uint8_t cmd[3];
    // set write mode and data len
    cmd[0] = MPSSE_DO_WRITE;
    cmd[1] = len - 1;
    cmd[2] = (len - 1) >> 8;

    check_error(ftdi_write_data(ftdi, cmd, 3));
    // send data out
    return ftdi_write_data(ftdi, data, len);
}
