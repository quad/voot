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

    Wed Aug  8 00:18:28 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        This is mainly just a note. The "util" and related util modules will
        be the only ones were the functions don't have to hold a standard
        naming paradigm. Mainly because, in the future, I don't want to
        rename "memcpy".

*/

#include "vars.h"
#include "util.h"

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
