/*  util.c

DESCRIPTION
    Contains a variety of miscellaneous utility functions.

    They're placed in here until they get large enough to warrant their own
    module.

COPYING
    See "COPYING" in the root directory of the distribution.

CHANGELOG
    Mon Aug  6 15:46:17 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        Imported, modified, and just generally added a timestamp when I
        created the libdc distribution.
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
