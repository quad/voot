/* This file is part of the Dreamcast function library.
 * Please see libdream.c for further details.
 *
 *  (C)2000 Jordan DeLong
 */

#ifndef __MOUSE_H
#define __MOUSE_H

/* mouse defines */
#define MOUSE_RIGHTBUTTON	(1<<1)
#define MOUSE_LEFTBUTTON	(1<<2)
#define MOUSE_SIDEBUTTON	(1<<3)

#define MOUSE_DELTA_CENTER	0x200

/* mouse condition structure */
typedef struct {
	uint16 buttons;
	uint16 dummy1;
	int16  dx;
	int16  dy;
	int16  dz;
	uint16 dummy2;
	uint32 dummy3;
	uint32 dummy4;
} mouse_cond_t;

int mouse_get_cond(uint8 addr, mouse_cond_t *cond);

#endif
