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
