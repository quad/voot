/*  biosfont.c

DESCRIPTION

    A accessor module for the BIOS font functions.

CHANGELOG

    Sat Mar  9 05:05:38 PST 2002    Scott Robinson <scott_vo@quadhome.com>
        First added this changelog entry. This code is an almost exact
        duplicate from Dan Potter's libdream's biosfont.c - except this
        version of the source checks the BIOS mutex.

*/

#include "vars.h"
#include "util.h"

#include "biosfont.h"

/* A little assembly that grabs the font address */
extern volatile uint8* bfont_get_address();
extern int32 bfont_lock();
extern void bfont_unlock();
asm("
_bfont_get_address:
    mov.l   syscall_b4, r0
    mov.l   @r0, r0
    jmp     @r0
    mov     #0, r1

_bfont_lock:
    mov.l   syscall_b4, r0
    mov.l   @r0, r0
    jmp     @r0
    mov     #1, r1

_bfont_unlock:
    mov.l   syscall_b4, r0
    mov.l   @r0, r0
    jmp     @r0
    mov     #2, r1

    .align 4
syscall_b4:
    .long   0x8c0000b4
");

volatile uint8 *bfont_address;

/* Pre-calculate the biosfont address. */
void bfont_init(void)
{
    if (!bfont_lock())
    {
        bfont_address = bfont_get_address();
        bfont_unlock();
    }
}

/* Given a character, find it in the BIOS font if possible */
volatile uint8* bfont_find_char(uint32 ch)
{
    int index = -1;

    /* 33-126 in ASCII are 1-94 in the font */
    if (ch >= 33 && ch <= 126)
        index = ch - 32;

    /* 160-255 in ASCII are 96-161 in the font */
    if (ch >= 160 && ch <= 255)
        index = ch - (160 - 96);

    /* Map anything else to a space */
    if (index == -1)
        index = 72 << 2;

    return bfont_address + index * 36;
}

/* Given a character, draw it into a buffer */
void bfont_draw(uint8 *buffer, uint32 bufwidth, uint32 c)
{
    uint8 ch[75];
    uint32 ch_base;
    uint16 word;
    uint32 x, y;

    if (!bfont_address)
        return;

    if (!bfont_lock())
    {
        memcpy(ch, (uint8 *) bfont_find_char(c), sizeof(ch));
        bfont_unlock();
        ch_base = 0;
    }
    else
        return;

    for (y=0; y<BFONT_CHAR_HEIGHT; )
    {
        /* Do the first row */
        word = (((uint16) ch[ch_base + 0]) << 4) | ((ch[ch_base + 1] >> 4) & 0x0f);
        for (x=0; x<BFONT_CHAR_WIDTH; x++)
        {
            if (word & (0x0800 >> x))
                *buffer = 0xff;

            buffer++;
        }
        buffer += bufwidth - BFONT_CHAR_WIDTH;
        y++;
        
        /* Do the second row */
        word = ((((uint16) ch[ch_base + 1]) << 8) & 0xf00) | ch[ch_base + 2];
        for (x=0; x<BFONT_CHAR_WIDTH; x++)
        {
            if (word & (0x0800 >> x))
                *buffer = 0xff;

            buffer++;
        }
        buffer += bufwidth - BFONT_CHAR_WIDTH;
        y++;
        
        ch_base += 3;
    }
}

void bfont_draw_str(uint8 *buffer, uint32 width, const char *str)
{
    while (*str)
        bfont_draw(buffer += BFONT_CHAR_WIDTH, width, *str++);
}
