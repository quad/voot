/*  assert.c

    $Id: assert.c,v 1.1 2002/06/12 00:03:43 quad Exp $

DESCRIPTION

    Assertion sub-system.

    In a state of panic, we can't be sure of the networking logic thus the
    module uses biosfont. If biosfont doesn't work, however, the screen is
    blanked to red.

*/

#include "vars.h"
#include "biosfont.h"
#include "system.h"
#include "util.h"
#include "printf.h"

#include "assert.h"

#define INDENT_BYTES        20
#define LINE_SPACING        24
#define VCON_FIRST_PIXEL    10

static uint32   vc_line;

/* TODO: This should be moved to a seperate video module. */

static void assert_clear (int16 r, int16 g, int16 b)
{
    uint16 *vram_s;
    uint16  pixel = (
                        ((r >> 3) << 11) |
                        ((g >> 2) << 5) |
                        ((b >> 3) << 0)
                   );
    int32   i;
    int32   vram_size;

    vram_s      = VIDEO_VRAM_START;
    vram_size   = 640 * 480;

    for (i = 0; i < vram_size; i++)
        vram_s[i] = pixel;
}

static void assert_puts (const char *in_str)
{
    uint16     *vram_index;
    static char null_string[] = "NULL";

    if (!in_str)
        in_str = null_string;

    /*
        STAGE: Position our text correctly.
        
        Assumes 640x - but then, so does bfont.
    */

    vram_index  = VIDEO_VRAM_START;
    vram_index += INDENT_BYTES;
    vram_index += (VCON_FIRST_PIXEL + (vc_line * LINE_SPACING)) * 640;

    /* STAGE: Make sure we handle '\n's. */

    while (*in_str)
    {
        if (*in_str == '\n')
        {
            vc_line++;
            in_str++;

            vram_index  = VIDEO_VRAM_START;
            vram_index += INDENT_BYTES;
            vram_index += (VCON_FIRST_PIXEL + (vc_line * LINE_SPACING)) * 640;

            continue;
        }

        /*
            STAGE: If the biosfont library is locked, just blank the screen.
            
            The fact we clear multiple times is not a bug. It's just
            laziness on the coder's part.
        */

        if (bfont_draw (vram_index += 12, 640, *in_str++))
            return assert_clear (100, 0, 0);
    }

    vc_line++;
}

void __assert (const char *module, int32 line, const char *expr, const char *func, uint32 extra)
{
    char line_msg[20];

    assert_puts ("*** ASSERTION FAILURE ***");

    /* STAGE: Module name. */

    assert_puts ("Module:");
    assert_puts (module);

    /* STAGE: Function the assertion occured in. */

    assert_puts ("Function:");
    assert_puts (func);

    /* STAGE: The line number of the assertion. */

    memset (line_msg, NULL, sizeof (line_msg));
    number (line_msg, line, 10, sizeof (line_msg), 0, N_LEFT);

    assert_puts ("Line Number:");
    assert_puts (line_msg);

    /* STAGE: The expression asserted. */

    assert_puts ("Expression:");
    assert_puts (expr);

    /* STAGE: The optional extra data passed in assert_x() */

    if (extra)
    {
        memset (line_msg, NULL, sizeof (line_msg));
        number (line_msg, extra, 16, sizeof (line_msg), 0, N_LEFT);

        assert_puts ("Message:");
        assert_puts (line_msg);
    }

    /* STAGE: Lock the system. */

    for (;;);
}
