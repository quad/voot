/*  ubc.c

    Initialize the UBC for breakpointing and the 1st_read.bin in memory.
*/

#include <string.h>
#include <dream.h>
#include "ubc.h"
#include "ubc-lowlevel.h"
#include "regs.h"
#include "serial.h"

/* The UBC Registers */
#define REGISTER(x)     (volatile unsigned x *)
#define _UBC_BASE       (0xFF200000)
#define _UBC_ASID_BASE  (0xFF000000)

#define UBC_R_BARA      (REGISTER(int)      (_UBC_BASE))
#define UBC_R_BAMRA     (REGISTER(char)     (_UBC_BASE + 0x04))
#define UBC_R_BBRA      (REGISTER(short)    (_UBC_BASE + 0x08))
#define UBC_R_BASRA     (REGISTER(char)     (_UBC_ASID_BASE + 0x14))
#define UBC_R_BRCR      (REGISTER(short)    (_UBC_BASE + 0x20))

/* The VBR Buffer - we better find out how large VO's actually is */
static unsigned char vbr_buffer[3072];

/* Borrowed from dcload-ip with andrewk's permission */
static void uint_to_string(unsigned int foo, unsigned char *bar)
{
    char hexdigit[16] = "0123456789abcdef";
    int i;

    for(i=7; i>=0; i--)
    {
        bar[i] = hexdigit[(foo & 0x0f)];
        foo = foo >> 4;
    }
    bar[8] = 0;
}

/*
SCIF Control Register - 0xFFE80008
SCIF Receive Register - 0xFFE80014
IP.BIN/UBC Load Point - 0x8C008000
VR ? Armor Location   - 0x8CCF7402
*/

#define LOADED_POINT        0x8C300000
#define REAL_LOAD_POINT     0x8C010000
#define P1_HEALTH           ((unsigned short *) 0x8CCF6284)
#define P2_HEALTH           ((unsigned short *) 0x8CCF7402)
#define BREAKPOINT          P1_HEALTH

static void dump_vbr_to_serial(void)
{
    ubc_serial_init(57600);
    ubc_serial_flush();

    ubc_serial_write_str("[UBC] Incoming VBR table for the next 3072 bytes!\n\r");
    ubc_serial_write_buffer(vbr_buffer, sizeof(vbr_buffer));
    ubc_serial_write_str("\r\n[UBC] VBR table sent. I hope you love me.\r\n");
}

static void check_reset_health(void)
{
    if (*BREAKPOINT < 500)
    {
        dump_vbr_to_serial();
        *BREAKPOINT = 1000;
    }
}

void ubc_break_work(void)
{
    check_reset_health();
}

int dc_main(void)
{
    unsigned long bin_size = 0xfeedbeef;
    char tmp_str[8];

    vid_init(vid_check_cable(), DM_640x480, PM_RGB565);
    vid_clear(0,100,0);

    /* Relocate the 1st_read.bin */
    bin_size = *((unsigned long *) REAL_LOAD_POINT);
    uint_to_string(bin_size, tmp_str);
    bfont_draw_str(vram_s+10*640+20, 640, "bin_size = ");
    bfont_draw_str(vram_s+50*640+20, 640, tmp_str);
    memmove((unsigned char *) REAL_LOAD_POINT, (unsigned char *) LOADED_POINT, bin_size);

    dbr_set(ubc_works);
    vbr_set(vbr_buffer);

    *UBC_R_BARA = (unsigned int) BREAKPOINT;
    *UBC_R_BAMRA = (1<<2);
    *UBC_R_BBRA = (1<<5) | (1<<4) | (1<<3) | (1<<2);
    *UBC_R_BRCR = (1<<10) | (1<<15) | 1;

    ubc_wait();
    ubc_wait();
    ubc_wait();
    ubc_wait();   

    vid_clear(50,0,50);

    disable_cache();
    (*(void (*)()) REAL_LOAD_POINT) ();

    vid_init(vid_check_cable(), DM_640x480, PM_RGB565);
    vid_clear(100,0,0);
    bfont_draw_str(vram_s+100*640+20, 640, "[UBC] DANGER! We're returned from the game!");
    while(1);
}
