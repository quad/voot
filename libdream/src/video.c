/* This file is part of the libdream Dreamcast function library.
 * Please see libdream.c for further details.
 *
 * (c)2000 Dan Potter
 */

#include "dream.h"

/*

This module supports video in 320x240 and 640x480, with bit depths of
555, 565, and 888. However, unless you use 640x480x565, you're kind
of on your own for most things. TA support only works with 640x480x565
and BIOS fonts work only with 565. There are other dependancies that assume
that sort of video mode elsewhere as well.

*/


/* Checks the attached cable type (to the A/V port). Returns
   one of the following:
     0 == VGA
     1 == (nothing)
     2 == RGB
     3 == Composite
   
   This is a direct port of Marcus' assembly function of the
   same name. */
int vid_check_cable() {
	unsigned long * porta = (unsigned long *)0xff80002c;
	
	*porta = (*porta & 0xfff0ffff) | 0x000a0000;
	
	// Read port8 and port9
	return (*((unsigned short*)(porta + 1)) >> 8) & 3;
}

/* The base address of the video chip */
unsigned long * videobase = (unsigned long *)0xa05f8000;

/* Base address of the video ram */
unsigned long * vram_l = (unsigned long *)0xa5000000;
unsigned short * vram_s = (unsigned short *)0xa5000000;

/* Pixel configuration (set by init_video) */
int vram_config = 0, disp_config = 0;

/* Size of the video memory (set by init_video) */
int vram_size = 0;

/* Cable type */
int vid_cable_type = -1;


/* Set up the video registers to the desired video mode (only
   640x480 supported right now). This function does not currently
   initialize all registers, but assumes that the boot ROM has set
   up reasonable defaults for syncs, etc.

   This is a fairly direct port of Marcus' assembly function of the 
   same name.
   
   Thanks to Chuck Mason for the 320x240 info.
   
   cable_type (0=VGA, 2=RGB, 3=Composite)
   pixel_mode (0=RGB555, 1=RGB565, 3=RGB888)
   
   */
void vid_init_320_240(int cable_type, int pixel_mode) {
	unsigned long * cvbsbase = (unsigned long *)0xa0702c00;
	int disp_lines, size_modulo, il_flag;
	
	/* Looks up bytes per pixel as a shift value */
	unsigned char bppshifttab[] = { 1, 1, 0, 2 };
	unsigned long bppshift = bppshifttab[pixel_mode];

	/* Save the video memory size */
	vram_size = 320*240;
	vram_config = pixel_mode;
	disp_config = DM_320x240;
	
	/* Set border color */
	vid_border_color(0, 0, 0);
	
	/* Set pixel clock and color mode */
	pixel_mode = (pixel_mode << 2) | 1;	/* Color mode and SD/DE */
	disp_lines = (240 / 2) << 1;
	if (cable_type == 0) {
		/* Double # of display lines for VGA and S-Video */
		disp_lines <<= 1;		/* Double line count */
		pixel_mode |= 0x800000;		/* Set clock double */
	}
	*(videobase + 0x11) = pixel_mode;
	*(videobase + 0x3a) |= 0x100;		/* Horizontal SD */

	/* Set video base address (right at the top) */
	vid_set_start(0);
	
	/* Set screen size and modulo, and interlace flag */
	il_flag = 0x100;	/* Set Video Output Enable */
	size_modulo = 0;
	disp_lines = (480 / 2) - 1;
	*(videobase + 0x17) = ((size_modulo+1) << 20) | (disp_lines << 10) 
	        | (((320/4) << bppshift) - 1);
	*(videobase + 0x34) = il_flag;

	/* Set vertical pos and border */
	*(videobase + 0x37) = (24 << 16) | (24 + disp_lines);
	*(videobase + 0x3c) = (24 << 16) | 24;
	
	/* Horizontal pos */
	*(videobase + 0x3b) = 0xa4;
	
	/* Set the screen size */
	*(videobase + 0x36) = (262<<16) | (857 << 0);

	/* Select RGB/CVBS */
	if (cable_type == 3)
		*cvbsbase = 3 << 8;
	else
		*cvbsbase = 0;
}

void vid_init_640_480(int cable_type, int pixel_mode) {
	unsigned long * cvbsbase = (unsigned long *)0xa0702c00;
	int disp_lines, size_modulo, il_flag;

	/* Looks up bytes per pixel as a shift value */
	unsigned char bppshifttab[] = { 1, 1, 0, 2 };
	unsigned long bppshift = bppshifttab[pixel_mode];

	/* Save the video memory size (number of pixels) */
	vram_size = (640*480);
	vram_config = pixel_mode;
	disp_config = DM_640x480;
	
	/* Set border color */
	vid_border_color(0, 0, 0);
	
	/* Set pixel clock and color mode */
	pixel_mode = (pixel_mode << 2) | 1;	/* Color mode and SD/DE */
	disp_lines = (240 / 2) << 1;
	if (cable_type == 0) {
		/* Double # of display lines for VGA */
		disp_lines <<= 1;		/* Double line count */
		pixel_mode |= 0x800000;		/* Set clock double */
	}
	*(videobase + 0x11) = pixel_mode;
	*(videobase + 0x3a) &= ~0x100;		/* Horizontal SD */
	
	/* Set video base address (right at the top) */
	vid_set_start(0);
	
	/* Set screen size and modulo, and interlace flag */
	il_flag = 0x100;		/* Video output enable */
	size_modulo = 0;
	if (cable_type != 0) {	/* non-VGA => interlace */
		/* Add one line to offset => display every other line */
		size_modulo = (640/4) << bppshift;
		il_flag |= 0x50;	/* Enable interlace */
	}
	*(videobase + 0x17) = ((size_modulo+1) << 20) | (disp_lines << 10) 
		| (((640/4) << bppshift) - 1);
	*(videobase + 0x34) = il_flag;
	
	/* Set vertical pos and border */
	*(videobase + 0x37) = (24 << 16) | (24 + disp_lines);
	*(videobase + 0x3c) = (24 << 16) | 24;
	
	/* Horizontal pos */
	*(videobase + 0x3b) = 0xa4;
	
	/* Set the screen size */
	*(videobase + 0x36) = (524 << 16) | (857 << 0);

	/* Select RGB/CVBS */
	if (cable_type == 3)
		*cvbsbase = 3 << 8;
	else
		*cvbsbase = 0;
}

void vid_init_800_608(int cable_type, int pixel_mode) {
	unsigned long * cvbsbase = (unsigned long *)0xa0702c00;
	int disp_lines, size_modulo, il_flag;

	/* Looks up bytes per pixel as a shift value */
	unsigned char bppshifttab[] = { 1, 1, 0, 2 };
	unsigned long bppshift = bppshifttab[pixel_mode];

	/* Save the video memory size (number of pixels) */
	vram_size = (800*608);
	vram_config = pixel_mode;
	disp_config = DM_800x608;
	
	/* Set border color */
	vid_border_color(0, 0, 0);
	
	/* Set pixel clock and color mode */
	pixel_mode = (pixel_mode << 2) | 1;	/* Color mode and SD/DE */
	disp_lines = ((608/2) / 2) << 1;
	if (cable_type == 0) {
		/* Double # of display lines for VGA */
		disp_lines <<= 1;		/* Double line count */
		pixel_mode |= 0x800000;		/* Set clock double */
	}
	*(videobase + 0x11) = pixel_mode;
	*(videobase + 0x3a) &= ~0x100;		/* Horizontal SD */
	
	/* Set video base address (right at the top) */
	vid_set_start(0);
	
	/* Set screen size and modulo, and interlace flag */
	il_flag = 0x100;		/* Video output enable */
	size_modulo = 0;
	if (cable_type != 0) {	/* non-VGA => interlace */
		/* Add one line to offset => display every other line */
		size_modulo = (800/4) << bppshift;
		il_flag |= 0x50;	/* Enable interlace */
	}
	*(videobase + 0x17) = ((size_modulo+1) << 20) | (disp_lines << 10) 
		| (((800/4) << bppshift) - 1);
	*(videobase + 0x34) = il_flag;
	
	/* Set vertical pos and border */
	*(videobase + 0x37) = (24 << 16) | (24 + disp_lines);
	*(videobase + 0x3c) = (24 << 16) | 24;
	
	/* Horizontal pos */
	*(videobase + 0x3b) = 0x44;
	
	/* Set the screen size */
	*(videobase + 0x36) = ((608+38) << 16) | (861 << 0);

	/* Select RGB/CVBS */
	if (cable_type == 3)
		*cvbsbase = 3 << 8;
	else
		*cvbsbase = 0;
}

void vid_init(int cable_type, int video_mode, int pixel_mode) {
	vid_cable_type = cable_type;
	switch(video_mode) {
		case DM_320x240:
			vid_init_320_240(cable_type, pixel_mode);
			break;
		case DM_640x480:
			vid_init_640_480(cable_type, pixel_mode);
			break;
		case DM_800x608:
			vid_init_800_608(cable_type, pixel_mode);
			break;
	}
}

/* Set display start address: assumes interlaced */
void vid_set_start(uint32 start) {
	volatile uint32 *regs = (uint32*)0xa05f0000;
	unsigned char bppshifttab[] = { 1, 1, 0, 2 };
	int shift = bppshifttab[vram_config];
	
	regs[0x8050/4] = start;

	/* In practice this doesn't matter if you're in VGA mode but it's
	   easier to just do it anyway. */
	switch (disp_config) {
		case DM_800x608:
			regs[0x8054/4] = start + (800 << shift);
			break;
		case DM_640x480:
			regs[0x8054/4] = start + (640 << shift);
			break;
		case DM_320x240:
			regs[0x8054/4] = start + (320 << shift);
			break;
	}
}

/* Sets the border color of the Dreamcast display */
void vid_border_color(int r, int g, int b) {
	*(videobase + 0x10) = (r << 16) | (g << 8) | (b << 0);
}

/* Clears the screen with a given color */
void vid_clear(int r, int g, int b) {
	switch (vram_config) {
		case PM_RGB555: {
			unsigned short pixel = ((r >> 3) << 10)
				| ((g >> 3) << 5)
				| ((b >> 3) << 0);
			int i;
			for (i=0; i<vram_size; i++)
				vram_s[i] = pixel;
		} break;
		case PM_RGB565: {
			unsigned short pixel = ((r >> 3) << 11)
				| ((g >> 2) << 5)
				| ((b >> 3) << 0);
			int i;
			for (i=0; i<vram_size; i++)
				vram_s[i] = pixel;
		} break;
		case PM_RGB888: {
			unsigned long pixel = (r << 16) | (g << 8) | (b << 0);
			int i;
			for (i=0; i<vram_size; i++)
				vram_l[i] = pixel;
		} break;
	}
}

/* Clears all of video memory as quickly as possible */
void vid_empty() {
	int i;
	for (i=0; i<8*1024*1024; i++)
		vram_l[i] = 0;
}

/* Waits for a vertical refresh to start. This is the period between
   when the scan beam reaches the bottom of the picture, and when it
   starts again at the top.
   
   Thanks to HeroZero for this info. */
void vid_waitvbl() {
	volatile unsigned long *vbl = videobase + 0x010c/4;
	while (!(*vbl & 0x01ff))
		;
	while (*vbl & 0x01ff)
		;
}

