/*  hud.c

    Code enabling a 2d overlay on a running VOOT system.
*/

#include "vars.h"
#include "system.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "biosfont.h"
#include "voot.h"
#include "util.h"
#include "printf.h"
#include <string.h>
#include "hud.h"

bool do_hud;
volatile uint16 *vid_mem;

#define HUD_CHAR_WIDTH      40
#define HUD_CHAR_HEIGHT     4
#define HUD_WIDTH           ((HUD_CHAR_WIDTH * BFONT_CHAR_WIDTH))
#define HUD_HEIGHT          ((HUD_CHAR_HEIGHT * BFONT_CHAR_HEIGHT))
#define HUD_STRIP_BUF_SIZE  (HUD_WIDTH + 1)

bool hud_initialized;
uint8 hud_char_buffer[HUD_CHAR_HEIGHT][HUD_CHAR_WIDTH];
uint8 *hud_vid_buffer;
uint16 *hud_vid_strip_buffer;

static bool hud_ok(void)
{
    return hud_initialized && do_hud;
}

void hud_init(void)
{
    /* STAGE: Malloc our off-screen buffer. */
    free(hud_vid_buffer);
    hud_vid_buffer = malloc((HUD_WIDTH * HUD_HEIGHT));
    memset(hud_vid_buffer, (HUD_WIDTH * HUD_HEIGHT), 0);

    /* STAGE: Malloc our off-screen strip buffer. */
    free(hud_vid_strip_buffer);
    hud_vid_strip_buffer = malloc(HUD_STRIP_BUF_SIZE / sizeof(uint32));

    /* DEBUG: Report whether our malloc()s worked. */
    if (hud_vid_buffer && hud_vid_strip_buffer)
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "hud ok\n");
    else
    {
        free(hud_vid_buffer);
        free(hud_vid_strip_buffer);
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "hud not ok\n");
        return;
    }

    /* STAGE: Locate the framebuffer. */
    vid_mem = (volatile uint16 *) (0xa5000000 + *((volatile uint32 *) 0xa05f8050));

    hud_initialized = TRUE;
}

void hud_write_line(const char *in_line)
{
    int32 line;

    if (!hud_ok())
        return;

    /* STAGE: Move the previous three lines "up." */
    for (line = 1; line < HUD_CHAR_HEIGHT; line++)
        strncpy(hud_char_buffer[line - 1], hud_char_buffer[line], HUD_CHAR_WIDTH);

    /* STAGE: Add new line to bottom of text buffer. */
    strncpy(hud_char_buffer[HUD_CHAR_HEIGHT - 1], in_line, HUD_CHAR_WIDTH);

    /* STAGE: Update off-screen buffer. */
    render_hud();
}

int32 hud_printf(const char *fmt, ...)
{
	va_list args;
	int32 i;
	char line[HUD_CHAR_WIDTH];

    if (!hud_ok())
        return 0;

    va_start(args, fmt);
    i = vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    if (i)
        hud_write_line(line);

    return i;
}

void render_hud(void)
{
    int32 line;

    if (!hud_ok())
        return;

    for (line = 0; line < HUD_CHAR_HEIGHT; line++)
        bfont_draw_str(hud_vid_buffer + (HUD_WIDTH * line), HUD_WIDTH, hud_char_buffer[line]);
}

void display_hud(void)
{
    uint32 x, y;

    if (!hud_ok())
        return;

    for (y = 0; y < HUD_HEIGHT; y++)
    {
        for (x = 0; x < HUD_WIDTH; x++)
        {
            if (hud_vid_buffer[(y * HUD_HEIGHT) + x])
                hud_vid_strip_buffer[x] = 0xFFFF;
            else
                hud_vid_strip_buffer[x] = 0x0;
        }

        sq_cpy((void *) (vid_mem + (y * 640)), (uint32 *) hud_vid_strip_buffer, HUD_STRIP_BUF_SIZE);
    }
}
