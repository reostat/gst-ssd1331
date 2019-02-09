#ifndef __ftdi_util_h__
#define __ftdi_util_h__

#include <ftdi.h>

// generic pin numbers
#define BUS0 (1 << 0)
#define BUS1 (1 << 1)
#define BUS2 (1 << 2)
#define BUS3 (1 << 3)
#define BUS4 (1 << 4)
#define BUS5 (1 << 5)
#define BUS6 (1 << 6)
#define BUS7 (1 << 7)

// MPSSE aliases
#define FT_SK (1 << 0)
#define FT_DO (1 << 1)
#define FT_DI (1 << 2)
#define FT_CS (1 << 3)
#define GPIO0 (1 << 4)
#define GPIO1 (1 << 5)
#define GPIO2 (1 << 6)
#define GPIO3 (1 << 7)

// 1 for output, 0 for input
#define PIN_DIRECTION (FT_SK | FT_DO | FT_CS | GPIO0)
// Initial pin state (mind that CS is active low)
#define PIN_STATE (FT_SK | FT_CS)

#define check_error(val) { \
        int __error_code__; \
        if((__error_code__ = val) < 0) \
            return __error_code__; \
    }

#define check_return(var, val) if ((var = val) < 0) return var;

int ftdi_set_10mhz_clk(struct ftdi_context *ftdi);

int ftdi_send_bytes(struct ftdi_context *ftdi, uint8_t *data, uint16_t len);

#endif
