/*  vconsole.c

    A really cheesy virtual console for myself. I'm tired of pre-calculating
    distances.
*/

#include <stdio.h>
#include <stdarg.h>
#include <dream.h>
#include "vconsole.h"

char printf_buf[1024];

unsigned short vc_line;

#define INDENT_BYTES        20
#define LINE_SPACING        22
#define VCON_FIRST_PIXEL    10

/* !!! Add in code for the console to loop back over on itself */
void vc_puts(char *in_str)
{
    unsigned short *vram_index;

    vram_index = vram_s;
    vram_index += INDENT_BYTES;

    /* Assumes 640x - but then, so does bfont */
    vram_index += (VCON_FIRST_PIXEL + (vc_line * LINE_SPACING)) * 640;
    
    bfont_draw_str(vram_index, 640, in_str);
    vc_line++;
}

/* Printf functionality */
int vc_printf(char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(printf_buf, fmt, args);
	va_end(args);

	vc_puts(printf_buf);

	return i;
}

void vc_clear(int r, int g, int b)
{
    vc_line = 0;
    vid_clear(r, g, b);
}
