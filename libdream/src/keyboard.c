/* This file is part of the libdream Dreamcast function library.
   Please see libdream.c for further details.
  
   keyboard.c
   (C)2000 Jordan DeLong and Dan Potter
*/

#include "dream.h"

/*

This module is an (almost) complete keyboard system. It handles key 
debouncing and queueing so you don't miss any pressed keys as long 
as you poll often enough. The only thing missing currently is key
repeat handling.

Ported from KallistiOS (Dreamcast OS) for libdream by Dan Potter

*/

/* get the condition structure for a keyboard at address addr.  return a
   -1 if an error occurs */
int kbd_get_cond(uint8 addr, kbd_cond_t *cond) {
	maple_frame_t frame;
	uint32 param[1];

	param[0] = MAPLE_FUNC_KEYBOARD;

	do {
		if (maple_docmd_block(MAPLE_COMMAND_GETCOND, addr, 1, param, &frame) == -1)
			return -1;
	} while (frame.cmd == MAPLE_RESPONSE_AGAIN);

	/* we get back func,condition */
	if (frame.cmd == MAPLE_RESPONSE_DATATRF
		&& (frame.datalen - 1) == sizeof(kbd_cond_t) / 4
		&& *((uint32 *) frame.data) == MAPLE_FUNC_KEYBOARD) {
		memcpy(cond, frame.data + 4, (frame.datalen - 1) * 4);
	} else {
		return -1;
	}

	return 0;
}

/* The key matrix array -- this is an array that lists the status of all
   keys on the keyboard. This is mainly used for key repeat and debouncing. */
uint8 kbd_matrix[256] = {0};

/* Shift key status */
uint8 kbd_shift_keys = 0;

/* The keyboard queue */
#define KBD_QUEUE_SIZE 16
int kbd_queue_active = 1;
uint32 kbd_head = 0, kbd_tail = 0;
uint16 kbd_queue[KBD_QUEUE_SIZE];

/* Turn keyboard queueing on or off. This is mainly useful if you want
   to use the keys for a game where individual keypresses don't mean
   as much as having the keys up or down. Setting this state to
   a new value will clear the queue. */
void kbd_set_queue(int active) {
	if (kbd_queue_active != active) {
		kbd_head = kbd_tail = 0;
	}
	kbd_queue_active = active;
}

/* Take a key scancode, encode it appropriately, and place it on the
   keyboard queue. At the moment we assume no key overflows. */
int kbd_enqueue(uint8 keycode) {
	static char keymap_noshift[] = {
	/*0*/	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
		'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
		'u', 'v', 'w', 'x', 'y', 'z',
	/*1e*/	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	/*28*/	13, 27, 8, 9, 32, '-', '=', '[', ']', '\\', 0, ';', '\'',
	/*35*/	'`', ',', '.', '/', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*46*/	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*53*/	0, '/', '*', '-', '+', 13, '1', '2', '3', '4', '5', '6',
	/*5f*/	'7', '8', '9', '0', '.', 0
	};
	static char keymap_shift[] = {
	/*0*/	0, 0, 0, 0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
		'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
		'U', 'V', 'W', 'X', 'Y', 'Z',
	/*1e*/	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	/*28*/	13, 27, 8, 9, 32, '_', '+', '{', '}', '|', 0, ':', '"',
	/*35*/	'~', '<', '>', '?', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*46*/	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*53*/	0, '/', '*', '-', '+', 13, '1', '2', '3', '4', '5', '6',
	/*5f*/	'7', '8', '9', '0', '.', 0
	};
	uint16 ascii;

	/* If queueing is turned off, don't bother */
	if (!kbd_queue_active)
		return 0;

	/* Figure out its key queue value */	
	if (kbd_shift_keys & (KBD_MOD_LSHIFT|KBD_MOD_RSHIFT))
		ascii = keymap_shift[keycode];
	else
		ascii = keymap_noshift[keycode];
	
	if (ascii == 0)
		ascii = ((uint16)keycode) << 8;

	/* Don't enque the error code */
	if (ascii == 0x0100)
		return 0;
		
	/* Ok... now do the enqueue */
	kbd_queue[kbd_head] = ascii;
	kbd_head = (kbd_head + 1) & (KBD_QUEUE_SIZE-1);
	
	return 0;
}	

/* Take a key off the key queue, or return -1 if there is none waiting */
int kbd_get_key() {
	int rv;

	/* If queueing isn't active, there won't be anything to get */
	if (!kbd_queue_active)
		return -1;
	
	/* Check available */
	if (kbd_head == kbd_tail)
		return -1;
	
	rv = kbd_queue[kbd_tail];
	kbd_tail = (kbd_tail + 1) & (KBD_QUEUE_SIZE-1);
	
	return rv;
}

/* Update the keyboard status; this will handle debounce handling as well as
   queueing keypresses for later usage. The key press queue uses 16-bit
   words so that we can store "special" keys as such. This needs to be called
   fairly periodically if you're expecting keyboard input. */
int kbd_poll(uint8 addr) {
	kbd_cond_t cond;
	int i;
	
	/* Poll the keyboard for currently pressed keys */
	if (kbd_get_cond(addr, &cond)) {
		return -1;
	}

	/* Check the shift state */
	kbd_shift_keys = cond.modifiers;
	
	/* Process all pressed keys */
	for (i=0; i<6; i++) {
		if (cond.keys[i] != 0) {
			int p = kbd_matrix[cond.keys[i]];
			kbd_matrix[cond.keys[i]] = 2;	/* 2 == currently pressed */
			if (!p)
				kbd_enqueue(cond.keys[i]);
		}
	}
	
	/* Now normalize the key matrix */
	for (i=0; i<256; i++) {
		if (kbd_matrix[i] == 1)
			kbd_matrix[i] = 0;
		else if (kbd_matrix[i] == 2)
			kbd_matrix[i] = 1;
	}
	
	return 0;
}






