/* This file is part of the Dreamcast function library.
 * Please see libdream.c for further details.
 *
 *  (C)2000 Jordan DeLong
 */

#include "dream.h"

/* Ported from KallistiOS for libdream by Dan Potter */


/* get the condition structure for a mouse at address addr. return a
   -1 if an error occurs. */
int mouse_get_cond(uint8 addr, mouse_cond_t *cond) {
	maple_frame_t frame;
	uint32 param[1];

	param[0] = MAPLE_FUNC_MOUSE;

	do {
		if (maple_docmd_block(MAPLE_COMMAND_GETCOND, addr, 1, param, &frame) == -1)
			return -1;
	} while (frame.cmd == MAPLE_RESPONSE_AGAIN);

	/* we get back func,condition */
	if (frame.cmd == MAPLE_RESPONSE_DATATRF
		&& (frame.datalen - 1) == sizeof(mouse_cond_t) / 4
		&& *((uint32 *) frame.data) == MAPLE_FUNC_MOUSE) {
		memcpy(cond, frame.data + 4, (frame.datalen - 1) * 4);
	} else {
		return -1;
	}

	cond->dx -= MOUSE_DELTA_CENTER;
	cond->dy -= MOUSE_DELTA_CENTER;
	cond->dz -= MOUSE_DELTA_CENTER;

	return 0;
}

