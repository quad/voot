/*  ubc.c

    The first stage loader of the UBC test/serial reinitializer:
    1) Load Virtual-On's 1ST_READ.BIN
    2) Load the 2nd stage into memory
    3) Run the 2nd stage
*/

#include <stdio.h>
#include <stdarg.h>
#include <dream.h>
#include "ubc.h"
#include "descramble.h"
#include "ubc-2ndstage.h"

#define FIRST_LOAD_POINT    0x8C300000
#define FIRST_RUN_POINT     0x8C010000
#define FIRST_LOAD_FILE     "/1st_read.bin"
#define IP_LOAD_POINT       0xAC008000
#define IP_RUN_POINT        0xAC008000
#define IP_BOOTSTRAP_POINT  0x8C00B800
#define IP_LOAD_FILE        "/ip.bin"

static char *ip_buffer = (char *) IP_LOAD_POINT;
static char *first_load_buffer = (char *) FIRST_LOAD_POINT;
static unsigned long *first_load_size = (unsigned long *) FIRST_RUN_POINT;

/* Printf functionality */
unsigned char printf_buf[2048];
int bfont_printf(uint16 *buffer, int width, char *fmt, ...) {
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(printf_buf, fmt, args);
	va_end(args);

	bfont_draw_str(buffer, width, printf_buf);

	return i;
}

/* GD-Rom BIOS calls... named mostly after Marcus' code. None have more
   than two parameters; R7 (fourth parameter) needs to describe
   which syscall we want. */

#define MAKE_SYSCALL(rs, p1, p2, idx) \
	uint32 *syscall_bc = (uint32*)0x8c0000bc; \
	int (*syscall)() = (int (*)())(*syscall_bc); \
	rs syscall((p1), (p2), 0, (idx));

/* Check drive status and get disc type */
static int gdc_get_drv_stat(void *param) { MAKE_SYSCALL(return, param, 0, 4); }

void boot_loader(void)
{
    int fd, bin_size;
    int do_descramble;
    unsigned char *first_temp_buffer;

    vid_clear(0,0,100);

    bfont_draw_str(vram_s+10*640+20, 640, "Initializing CD subsystem...");
    cdrom_reinit();

    bfont_draw_str(vram_s+30*640+20, 640, "Initializing ISO subsystem...");
    iso_init();

    bfont_draw_str(vram_s+50*640+20, 640, "Trying: GD");
    fd = iso_open_gdrom(FIRST_LOAD_FILE, O_RDONLY);
    do_descramble = 0;
    if (!fd)
    {
        /* Must not be a GD-ROM */
        bfont_draw_str(vram_s+70*640+20, 640, "Trying: CD");
        fd = iso_open(FIRST_LOAD_FILE, O_RDONLY);
        do_descramble = 1;
    }

    if (!fd)
    {
        bfont_draw_str(vram_s+100*640+20, 640, "What the hell kinda media is this?!\n");
    }
    else
    {
        bin_size = iso_total(fd);

        if (do_descramble)
        {
            /* KEEP IN MIND THIS DOESN'T WORK YET, FOR SOME REASON! */
            vid_clear(100,0,0);
            first_temp_buffer = first_load_buffer + (bin_size * 3);
            /* Descramblation code goes here */
            vid_clear(0,100,0);
            bfont_printf(vram_s+20*640+20, 640, "first_load_buffer = %x", first_load_buffer);
            bfont_printf(vram_s+50*640+20, 640, "first_temp_buffer = %x", first_temp_buffer);
            iso_read(fd, first_temp_buffer, bin_size);
            descramble(first_temp_buffer, first_load_buffer, bin_size);
            vid_clear(0,0,100);
            bfont_printf(vram_s+50*640+20, 640, "STUMBLE YOU MIGHT FALL");
            bfont_printf(vram_s+50*640+20, 640, "%x", first_load_buffer[0]);
            bfont_printf(vram_s+80*640+20, 640, "%x", first_load_buffer[4]);
        }
        else
        {
            bfont_printf(vram_s+70*640+20, 640, "Loading %s (GD-style/%u)...", FIRST_LOAD_FILE, bin_size);
            iso_read(fd, first_load_buffer, bin_size);
        }

        iso_close(fd);
        memcpy(ip_buffer, stage_two_bin, sizeof(stage_two_bin));
        bfont_draw_str(vram_s+90*640+20, 640, "Executing 2nd Stage!");
        *first_load_size = bin_size;
        (*(void (*)()) IP_RUN_POINT) ();
    }
}

void wait_for_gdrom(void)
{
    unsigned int params[4];

    cdrom_reinit();

    params[0] = params[1] = 255;
    while(1)
    {
        gdc_get_drv_stat(params);
        if (params[0] == 1 && params[1] == 0x80)
            break;

        vid_waitvbl();
    }
}

void dc_main(void)
{
    vid_init(vid_check_cable(), DM_640x480, PM_RGB565);
    vid_clear(0,0,0);
    cdrom_init();

    bfont_draw_str(vram_s+30*640+20, 640, "Insert the GD-ROM.");
    wait_for_gdrom();

    boot_loader();

    bfont_draw_str(vram_s+300*640+20, 640, "We're finished. I guess something bad happened.");
    while(1);
}
