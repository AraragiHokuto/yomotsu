/* early vga console driver */

#include <kern/console.h>
#include <kern/string.h>
#include <kern/macrodef.h>
#include <kern/asm.h>

#define	DIMX	80
#define	DIMY	25

static uint	pos_x, pos_y;

uint
vga_text_dim_x(void)
{
	return DIMX;
}

uint
vga_text_dim_y(void)
{
	return DIMY;
}

static void
__update_cursor(uint x, uint y)
{
	u16 pos = y * DIMX + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, pos & 0xFF);
	outb(0x3D4, 0x0E);
	outb(0x3D5, pos >> 8);
}

void
vga_text_scroll(uint lc)
{
	ASSERT(lc <= DIMY);

	u16 *dst	= (u16*)0xFFFFFFFF800B8000;
	u16 *src	= dst + (lc * DIMX);
	u16 *end	= dst + (DIMX * DIMY);

	kmemmov(dst, src, (end - src) * sizeof(u16));

	src = dst + ((DIMY - lc) * DIMX);
	kmemset(src, 0, (end - src) * sizeof(u16));

	pos_y -= lc;
	__update_cursor(pos_x, pos_y);
}

void
vga_text_clear(void)
{
	u16 *dst	= (u16*)0xFFFFFFFF800B8000;
	kmemset(dst, 0, DIMX * DIMY * sizeof(u16));
	pos_x	= pos_y	= 0;
}

static void
__newline(void)
{
	++pos_y;
	pos_x = 0;

	if (pos_y >= DIMY) {
		vga_text_scroll(1);
	}
}

#define	TAB_WIDTH	8

void
vga_text_write(uint fg, uint bg,
	       const char *str, size_t len)
{
	u16 *vmem	= (u16*)0xFFFFFFFF800B8000;
	u16 color	= fg | bg << 8;

	for (size_t i = 0; i < len; ++i) {
		switch (str[i]) {
		case '\n':
			__newline();
			break;
		case '\t':
			if (pos_x == 0) {
				pos_x = TAB_WIDTH;
			} else {
				/* round X to next tabstop */
				--pos_x;
				pos_x /= TAB_WIDTH;
				++pos_x;
				pos_x *= TAB_WIDTH;
			}
			break;
		default:
			vmem[pos_y * DIMX + pos_x] = (color << 8) + str[i];
			if (++pos_x >= DIMX) {
				__newline();
			}
			break;
		}
	}
	__update_cursor(pos_x, pos_y);
}

static con_driver_t	vga_text_driver;

con_driver_t *
vga_text_init(void)
{
	pos_x = pos_y = 0;
	vga_text_driver.dim_x = vga_text_dim_x;
	vga_text_driver.dim_y = vga_text_dim_y;
	vga_text_driver.write = vga_text_write;
	vga_text_driver.scroll = vga_text_scroll;
	vga_text_driver.clear = vga_text_clear;
	return &vga_text_driver;
}
