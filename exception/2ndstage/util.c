/*  util.c

    Yeah, everyone needs a utility module.
*/

#include "vars.h"

/* Borrowed from dcload-ip with andrewk's permission */
void uint_to_string(uint32 foo, uint8 *bar)
{
    char hexdigit[16] = "0123456789abcdef";
    int32 i;

    for(i=7; i>=0; i--) {
    	bar[i] = hexdigit[(foo & 0x0f)];
	    foo = foo >> 4;
    }

    bar[8] = 0;
}

/* Borrowed from libdream. */
void vid_waitvbl(void)
{
	volatile uint32 *vbl = ((volatile uint32 *) 0xa05f8000) + 0x010c / sizeof(uint32);

	while (!(*vbl & 0x01ff))
		;
	while (*vbl & 0x01ff)
		;
}
