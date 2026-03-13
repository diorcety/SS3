#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "gfx.h"
#include "st7789.h"

// Static inline functions
static GFXINLINE void init_board(GDisplay *g)
{
    board_lcd_init();
}

static GFXINLINE void post_init_board(GDisplay *g)
{
    (void) g;
}

static GFXINLINE void setpin_reset(GDisplay *g, gI8 state)
{
    board_lcd_reset(state);
}

static GFXINLINE void acquire_bus(GDisplay *g)
{
    (void) g;
}

static GFXINLINE void release_bus(GDisplay *g)
{
    (void) g;
}

static GFXINLINE void acquire_sem()
{
    /* Code here*/
}

static GFXINLINE void release_sem()
{
    /* Code here*/
}

static GFXINLINE void write_cmd(GDisplay *g, uint8_t cmd)
{
    board_lcd_write_cmd(cmd);
}

static GFXINLINE void write_data(GDisplay *g, uint16_t data)
{
    board_lcd_write_data(data);
}

static GFXINLINE void write_data_byte(GDisplay *g, uint8_t data)
{
    board_lcd_write_data_byte(data);
}

static GFXINLINE void write_data_byte_repeat(GDisplay *g, uint16_t data, int point_num)
{
    board_lcd_write_data_byte_repeat(data, point_num);
}

static GFXINLINE void write_cmddata(GDisplay *g, uint8_t cmd, uint32_t data)
{
    board_lcd_write_cmddata(cmd, data);
}

static GFXINLINE void write_datas(GDisplay *g, uint8_t *data, uint16_t length)
{
    board_lcd_write_datas(data, length);
}

static GFXINLINE void blit_area(GDisplay *g)
{
    const uint16_t	*buffer;
    buffer = (const uint16_t *)g->p.ptr;
    buffer += g->p.y1 * g->p.x2 + g->p.x1;	// The buffer start position
    board_lcd_blit_area(g->p.x, g->p.y, buffer, g->p.cx, g->p.cy);
}

static GFXINLINE void set_backlight(GDisplay *g, uint16_t data)
{
    board_lcd_set_backlight(data);
}

static GFXINLINE void set_viewport(GDisplay *g)
{
    #define MAKEWORD(b1, b2, b3, b4) ((uint32_t) ((b1) | ((b2) << 8) | ((b3) << 16) | ((b4) << 24)))
    write_cmddata(g, ST7789_CASET, MAKEWORD( ((g->p.x) >> 8), (g->p.x) & 0xFF, ((g->p.x + g->p.cx - 1) >> 8), (g->p.x + g->p.cx - 1) & 0xFF));
    write_cmddata(g, ST7789_RASET, MAKEWORD( ((g->p.y) >> 8), (g->p.y) & 0xFF, ((g->p.y + g->p.cy - 1) >> 8), (g->p.y + g->p.cy - 1) & 0xFF));
    write_cmd (g, ST7789_RAMWR);
    #undef MAKEWORD
}

#endif /* _GDISP_LLD_BOARD_H */