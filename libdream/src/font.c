/* This file is part of the Dreamcast function library.
 * Please see libdream.c for further details.
 *
 * (c)2000 Dan Potter
 */

#include <stdio.h>
#include <stdarg.h>
#include "dream.h"

/* You must call this function to setup a font to use for
   the rest of the drawing functions. The font should be composed
   of 256 bitmapped 8-pixel-wide rows (1 byte per row). */
int		font_height = 0;
unsigned char *	font_bm = NULL;
void font_set(unsigned char * fbm, int fh) {
	font_height = fh;
	font_bm = fbm;
}

/* Draws a single character using the loaded font. Assumes a
   PM_RGB565 display mode. */
void draw_char(int x1, int y1, unsigned short color, int ch) {
	int offs = ch * font_height;
	unsigned short * out = vram_s + y1 * 640 + x1;
	int x, y;
	
	for (y=0; y<font_height; y++) {
		int mask = 0x80;
		for (x=0; x<8; x++) {
			unsigned short pixel = 0;
			if (font_bm[offs] & mask) {
				pixel = color;
				out[x] = pixel;
			}

			mask >>= 1;
		}
		out += 640; offs++;
	}
}

/* Draws a single character using the loaded font; the pixels
    will be repeated by two times in each direction. Assumes a
    PM_RGB565 display mode. */
void draw_char_2(int x1, int y1, unsigned short color, int ch) {
	int offs = ch * font_height;
	unsigned short * out = vram_s + y1 * 640 + x1;
	int x, y, ex, ey;
	
	for (y=0; y<font_height*2; y++) {
		int mask = 0x80;
		ey = y / 2;
		for (x=0; x<8*2; x++) {
			unsigned short pixel = 0;
			ex = x / 2;
			if (font_bm[offs] & mask) {
				pixel = color;
				out[x] = pixel;
			}

			if (x && !(x % 2)) mask >>= 1;
		}
		out += 640;
		if (y && !(y % 2)) offs++;
	}
}

/* Draws a single character using the loaded font; the pixels
    will be repeated by four times in each direction. Assumes a
    PM_RGB565 display mode. */
void draw_char_4(int x1, int y1, unsigned short color, int ch) {
	int offs = ch * font_height;
	unsigned short * out = vram_s + y1 * 640 + x1;
	int x, y, ex, ey;
	
	for (y=0; y<font_height*4; y++) {
		int mask = 0x80;
		ey = y / 4;
		for (x=0; x<8*4; x++) {
			unsigned short pixel = 0;
			ex = x / 4;
			if (font_bm[offs] & mask) {
				pixel = color;
				out[x] = pixel;
			}

			if (x && !(x % 4)) mask >>= 1;
		}
		out += 640;
		if (y && !(y % 4)) offs++;
	}
}

/* Draws an entire string using the loaded font. */
void draw_string(int x1, int y1, unsigned short color, char *str) {
	while (*str) {
		draw_char(x1, y1, color, *str);
		x1 += 8; str++;
	}
}

/* Draws an entire string using the loaded font using the _2 function. */
void draw_string_2(int x1, int y1, unsigned short color, char *str) {
	while (*str) {
		draw_char_2(x1, y1, color, *str);
		x1 += 8*2; str++;
	}
}

/* Draws an entire string using the loaded font using the _4 function. */
void draw_string_4(int x1, int y1, unsigned short color, char *str) {
	while (*str) {
		draw_char_4(x1, y1, color, *str);
		x1 += 8*4; str++;
	}
}

/* Uses printf() style formatting to draw a string. */
void draw_stringf(int x1, int y1, unsigned short color, char *fmt, ...) {
	va_list		args;
	char		pbuf[2048];

	va_start(args, fmt);
	vsprintf(pbuf, fmt, args);
	va_end(args);

	draw_string(x1, y1, color, pbuf);
}

/* Uses printf() style formatting to draw a string; uses the _2 function. */
void draw_stringf_2(int x1, int y1, unsigned short color, char *fmt, ...) {
	va_list		args;
	char		pbuf[2048];

	va_start(args, fmt);
	vsprintf(pbuf, fmt, args);
	va_end(args);

	draw_string_2(x1, y1, color, pbuf);
}

/* Uses printf() style formatting to draw a string; uses the _4 function. */
void draw_stringf_4(int x1, int y1, unsigned short color, char *fmt, ...) {
	va_list		args;
	char		pbuf[2048];

	va_start(args, fmt);
	vsprintf(pbuf, fmt, args);
	va_end(args);

	draw_string_4(x1, y1, color, pbuf);
}

