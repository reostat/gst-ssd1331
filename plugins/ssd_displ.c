#include "ftdi_util.h"
#include "ssd_displ.h"

#define ssd_error_return(code, str) do {  \
            ssd->error_str = str;         \
            return code;                  \
        } while(0);

static int ssd_connect(ssd_context *ssd, int vid, int pid)
{
    // Interface A is used for FPGA EEPROM, we need interface B
    check_error(ftdi_set_interface(ssd->ftdi_handle, INTERFACE_B));
    check_error(ftdi_usb_open(ssd->ftdi_handle, vid, pid));
    return 0;
}

static int ssd_disconnect(ssd_context *ssd)
{
    if (!ssd->ftdi_connected) return 0;
    ftdi_disable_bitbang(ssd->ftdi_handle);
    ftdi_usb_reset(ssd->ftdi_handle);
    ftdi_usb_close(ssd->ftdi_handle);
    ssd->ftdi_connected = 0;
    return 0;
}

static int setup_mpsse(ssd_context *ssd) {
    check_error(ftdi_usb_reset(ssd->ftdi_handle));
    check_error(ftdi_usb_purge_buffers(ssd->ftdi_handle));
    check_error(ftdi_set_latency_timer(ssd->ftdi_handle, 1));
    check_error(ftdi_set_bitmode(ssd->ftdi_handle, 0x0, BITMODE_RESET));
    check_error(ftdi_set_bitmode(ssd->ftdi_handle, 0x0, BITMODE_MPSSE));
    ssd->ftdi_connected = 1;
    usleep(100);

    int res;
    uint8_t data[2];

    data[0] = LOOPBACK_START;
    check_error(ftdi_write_data(ssd->ftdi_handle, data, 1));
    check_return(res, ftdi_read_data(ssd->ftdi_handle, data, 2));
    if (res > 0) {
        ssd_error_return(-10, "Unexpected data read after loopback start");
    }

    data[0] = 0xAB; // bogus command
    check_error(ftdi_write_data(ssd->ftdi_handle, data, 1));
    check_return(res, ftdi_read_data(ssd->ftdi_handle, data, 2));
    if (res != 2) {
        ssd_error_return(-11, "Expected 2 byte response to bogus command");
    } else if (data[0] != 0xFA || data[1] != 0xAB) {
        ssd_error_return(-12, "Error in synchronizing MPSSE");
    }

    data[0] = LOOPBACK_END;
    check_error(ftdi_write_data(ssd->ftdi_handle, data, 1));
    check_return(res, ftdi_read_data(ssd->ftdi_handle, data, 2));
    if (res > 0) {
        ssd_error_return(-13, "Unexpected data read after loopback end");
    }

    check_error(ftdi_set_10mhz_clk(ssd->ftdi_handle));

    return 0;
}

ssd_context *ssd_new(void)
{
    struct ftdi_context *ftdi;
    if ((ftdi = ftdi_new()) == NULL)
        return NULL;
    
    ssd_context *ssd = malloc(sizeof(ssd_context));
    if (ssd == NULL) {
        ftdi_free(ftdi);
        return NULL;
    }

    ssd->ftdi_handle = ftdi;
    ssd->error_str = NULL;
    ssd->ftdi_connected = 0;
    ssd->transfer_started = 0;

    return ssd;
}

int ssd_open(ssd_context *ssd, int vid, int pid)
{
    if (ssd == NULL)
        return -1;
    
    check_error(ssd_connect(ssd, vid, pid));

    int err;
    if ((err = setup_mpsse(ssd)) < 0) {
        ssd_disconnect(ssd);
        return err;
    }
    return 0;
}

void ssd_free(ssd_context *ssd)
{
    if (ssd == NULL)
        return;
    ssd_disconnect(ssd);
    ftdi_free(ssd->ftdi_handle);
    free(ssd);
}

static int ssd_set_pins(ssd_context *ssd, uint8_t pin_init)
{
    uint8_t bytes[3];
    // set initial pin state and direction
    bytes[0] = SET_BITS_LOW;
    bytes[1] = pin_init;
    bytes[2] = PIN_DIRECTION;
    return ftdi_write_data(ssd->ftdi_handle, bytes, 3);
}

int ssd_begin_pixel_transfer(ssd_context *ssd)
{
    ssd->transfer_started = 1;
    // Assert CS, pull GPIO0 high for data
    return ssd_set_pins(ssd, (PIN_STATE | GPIO0) & ~FT_CS);
}

int ssd_begin_command_transfer(ssd_context *ssd)
{
    ssd->transfer_started = 1;
    // pull GPIO0 low for command, assert CS
    return ssd_set_pins(ssd, PIN_STATE & ~GPIO0 & ~FT_CS);
}

int ssd_end_transfer(ssd_context *ssd)
{
    ssd->transfer_started = 0;
    // deassert CS after data transfer
    return ssd_set_pins(ssd, PIN_STATE | FT_CS);
}

int ssd_send_pixels(ssd_context *ssd, uint8_t *data, uint16_t len)
{
    if (!ssd->transfer_started) {
        check_error(ssd_begin_pixel_transfer(ssd));
        check_error(ftdi_send_bytes(ssd->ftdi_handle, data, len));
        return ssd_end_transfer(ssd);
    } else {
        return ftdi_send_bytes(ssd->ftdi_handle, data, len);
    }
}

int ssd_send_command(ssd_context *ssd, uint8_t *data, uint16_t len)
{
    if (!ssd->transfer_started) {
        check_error(ssd_begin_command_transfer(ssd));
        check_error(ftdi_send_bytes(ssd->ftdi_handle, data, len));
        return ssd_end_transfer(ssd);
    } else {
        return ftdi_send_bytes(ssd->ftdi_handle, data, len);
    }
}

char *ssd_get_error_string (ssd_context *ssd)
 {
    if (ssd == NULL) return "";

    return ssd->error_str != NULL
        ? ssd->error_str
        : ftdi_get_error_string(ssd->ftdi_handle);
 }