/* This file is part of the Dreamcast function library.
 * Please see libdream.c for further details.
 *
 * (c)2000 Dan Potter
 */

#ifndef __DREAM_H
#define __DREAM_H

/*********** General setup *****/
#ifndef NULL
#define NULL (void*)0
#endif

typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef signed long int32;
typedef signed short int16;
typedef signed char int8;

/*********** libdream.c ********/
int dc_setup(int video_mode, int pixel_mode);
int dc_setup_quiet(int video_mode, int pixel_mode);

/*********** video.c ***********/

extern unsigned long *vram_l;
extern unsigned short *vram_s;
extern int vram_config;
extern int vram_size;
extern int vid_cable_type;

#define CT_VGA		0
#define CT_RGB		2
#define CT_COMPOSITE	3
int vid_check_cable();

#define PM_RGB555	0
#define PM_RGB565	1
#define PM_RGB888	3
#define DM_640x480	0
#define DM_320x240	1
#define DM_800x608	2
void vid_init(int cable_type, int disp_mode, int pixel_mode);

void vid_set_start(uint32 start);
void vid_border_color(int r, int g, int b);
void vid_clear(int r, int g, int b);
void vid_empty();
void vid_waitvbl();

/********** biosfont.c *********/
void* bfont_find_char(int ch);
void bfont_draw(uint16 *buffer, int bufwidth, int c);
void bfont_draw_str(uint16 *buffer, int bufwidth, char *str);
int bfont_printf(uint16 *buffer, int width, char *fmt, ...);

/************ serial.c *********/
void serial_disable();

void serial_init(int baud);
void serial_write(int c);
void serial_flush();
void serial_write_buffer(uint8 *data, int len);
void serial_write_str(char *str);
int serial_read();
int serial_printf(char *fmt, ...);
#define printf serial_printf


/************ font.c ***********/
void font_set(unsigned char * fbm, int fh);
void draw_char(int x1, int y1, unsigned short color, int ch);
void draw_char_2(int x1, int y1, unsigned short color, int ch);
void draw_char_4(int x1, int y1, unsigned short color, int ch);
void draw_string(int x1, int y1, unsigned short color, char *str);
void draw_string_2(int x1, int y1, unsigned short color, char *str);
void draw_string_4(int x1, int y1, unsigned short color, char *str);
void draw_stringf(int x1, int y1, unsigned short color, char *fmt, ...);
void draw_stringf_2(int x1, int y1, unsigned short color, char *fmt, ...);
void draw_stringf_4(int x1, int y1, unsigned short color, char *fmt, ...);

/************ spu.c ************/
#define SM_16BIT	0		/* signed 16-bit words */
#define SM_8BIT		1		/* signed 8-bit bytes */
#define SM_ADPCM	3		/* ADPCM compressed samples */
#define SM_STATIC	(0xff << 1)	/* Static test pattern; can be used
					   for a really simple chip-player =) */
#define SMP_BASE	0x10000

void snd_ram_write_wait();
void snd_load_arm(void* src, int size);
void snd_stop_arm();
void snd_init();
void snd_load(void *src, int dest, int len);

/************ cdfs.c ************/
#include "cdrom.h"
#include "fs_iso9660.h"

/************ maple.c ***********/
#include "maple.h"

/************ controller.c ******/
#include "controller.h"

/************ keyboard.c ********/
#include "keyboard.h"

/************ vmu.c *************/
#include "vmu.h"

/************ mouse.c ***********/
#include "mouse.h"

/************ timer.c ***********/
#include "timer.h"
#define sleep timer_sleep

#endif	// __DREAM_H




