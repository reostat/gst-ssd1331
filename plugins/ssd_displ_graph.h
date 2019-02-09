//--------------------------------------------------
// Helpers for SSD1331 graphic acceleration commands
//--------------------------------------------------

#ifndef __ssd_displ_graph_h__
#define __ssd_displ_graph_h__

#include "ssd_displ.h"

// Point on display: x = [0..95], y = [0..64]
typedef struct {
    uint8_t x, y;
} point_t;

point_t *set_point(point_t *p, uint8_t x, uint8_t y);

// Line on display
typedef struct {
    point_t start, end;
} line_t;

line_t *set_line(line_t *l, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
line_t *set_line_points(line_t *l, point_t const *start, point_t const *end);

// RGB565 color: 5 bit red, 6 bit green, 5 bit blue
typedef struct {
    uint8_t r, g, b;
} color565_t;

color565_t *set_color(color565_t *c, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Clear window
 */
int ssd_clear_window(ssd_context *ssd, line_t const *diagonal);

int ssd_clear_display(ssd_context *ssd);

/**
 * @brief Draw a line
 */
int ssd_draw_line(ssd_context *ssd, line_t const *line, color565_t const *color);

int ssd_toggle_fill_mode(ssd_context *ssd, uint8_t mode);
int ssd_draw_box(ssd_context *ssd, line_t const *box, color565_t const *border_color, color565_t const *fill_color);

#endif