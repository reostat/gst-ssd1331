#include "ftdi_util.h"
#include "ssd_displ.h"
#include "ssd_displ_graph.h"

// SSD1331 graphic commands
#define CMD_DRAW_LINE       0x21
#define CMD_DRAW_BOX        0x22
#define CMD_CLEAR_WINDOW    0x25
#define CMD_SET_FILL        0x26

#define TO_R(val) (val << 1)
#define TO_G(val) (val)
#define TO_B(val) (val << 1)

static int fill_line_data(uint8_t *buf, line_t const *line)
{
    int len = 0;
    buf[len++] = line->start.x;
    buf[len++] = line->start.y;
    buf[len++] = line->end.x;
    buf[len++] = line->end.y;
    return len;
}

static int fill_color_data(uint8_t *buf, color565_t const *color)
{
    int len = 0;
    buf[len++] = TO_R(color->r);
    buf[len++] = TO_G(color->g);
    buf[len++] = TO_B(color->b);
    return len;
}

int ssd_clear_window(ssd_context *ssd, line_t const *diagonal)
{
    int cmd_len = 5;
    uint8_t cmd[cmd_len];
    cmd_len = 0;
    cmd[cmd_len++] = CMD_CLEAR_WINDOW;
    cmd_len += fill_line_data(cmd + cmd_len, diagonal);
    return ssd_send_command(ssd, cmd, cmd_len);
}

int ssd_clear_display(ssd_context *ssd)
{
    line_t line = {{0, 0}, {MAX_X, MAX_Y}};
    return ssd_clear_window(ssd, &line);
}

int ssd_draw_line(ssd_context *ssd, line_t const *line, color565_t const *color)
{
    int cmd_len = 8;
    uint8_t cmd[cmd_len];
    cmd_len = 0;
    cmd[cmd_len++] = CMD_DRAW_LINE;
    cmd_len += fill_line_data(cmd + cmd_len, line);
    cmd_len += fill_color_data(cmd + cmd_len, color);
    return ssd_send_command(ssd, cmd, cmd_len);
}

int ssd_toggle_fill_mode(ssd_context *ssd, uint8_t mode)
{
    u_int8_t cmd[2];
    cmd[0] = CMD_SET_FILL;
    cmd[1] = mode != 0;
    return ssd_send_command(ssd, cmd, 2);
}

int ssd_draw_box(ssd_context *ssd, line_t const *box, color565_t const *border_color, color565_t const *fill_color)
{
    int cmd_len = 11;
    uint8_t cmd[cmd_len];
    cmd_len = 0;
    cmd[cmd_len++] = CMD_DRAW_BOX;
    cmd_len += fill_line_data(cmd + cmd_len, box);
    cmd_len += fill_color_data(cmd + cmd_len, border_color);
    if (fill_color != NULL) {
        cmd_len += fill_color_data(cmd + cmd_len, fill_color);
        check_error(ssd_toggle_fill_mode(ssd, 1));
    } else {
        cmd_len += fill_color_data(cmd + cmd_len, border_color);
    }
    check_error(ssd_send_command(ssd, cmd, cmd_len));
    if (fill_color != NULL) {
        check_error(ssd_toggle_fill_mode(ssd, 0));
    }
    return 0;
}

point_t *set_point(point_t *p, uint8_t x, uint8_t y) {
    p->x = x;
    p->y = y;
    return p;
}

line_t *set_line(line_t *l, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    l->start.x = x1; l->start.y = y1;
    l->end.x = x2; l->end.y = y2;
    return l;
}

line_t *set_line_points(line_t *l, point_t const *start, point_t const *end) {
    return set_line(l, start->x, start->y, end->x, end->y);
}

color565_t *set_color(color565_t *c, uint8_t r, uint8_t g, uint8_t b) {
    c->r = r; c->g = g; c->b = b;
    return c;
}