#ifndef __ssd_displ_h__
#define __ssd_displ_h__

#include <ftdi.h>

#define DISPL_WIDTH 96
#define DISPL_HEIGHT 64
#define MAX_X 95
#define MAX_Y 63

typedef struct
{
    struct ftdi_context *ftdi_handle;
    char *error_str;
    uint8_t ftdi_connected;
    uint8_t transfer_started;
} ssd_context;

/**
 * @brief Create and initialize new ssd_context. Created context has to be released with ssd_free.
 */
ssd_context *ssd_new(void);

int ssd_open(ssd_context *ssd, int vid, int pid);

int ssd_begin_pixel_transfer(ssd_context *ssd);
int ssd_begin_command_transfer(ssd_context *ssd);
int ssd_end_transfer(ssd_context *ssd);

int ssd_send_pixels(ssd_context *ssd, uint8_t *data, uint16_t len);

int ssd_send_command(ssd_context *ssd, uint8_t *data, uint16_t len);

/**
 * @brief Release ssd_context created by ssd_new.
 */
void ssd_free(ssd_context *ssd);

char *ssd_get_error_string (ssd_context *ssd);

#endif
