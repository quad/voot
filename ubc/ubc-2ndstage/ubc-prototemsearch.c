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
unsigned char vbr_buffer[3072];

/* Borrowed from dcload-ip with andrewk's permission */
void uint_to_string(unsigned int foo, unsigned char *bar)
{
    char hexdigit[16] = "0123456789abcdef";
    int i;

    for(i=7; i>=0; i--) {
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

#define BREAKPOINT_OLD  ((unsigned int *) (0x8CCF7402))
#define BREAKPOINT      ((unsigned int *) (0x8C2397A2))

void ubc_break_work(void)
{
    char num_buffer[10];
    volatile short *p1_health = (unsigned short *) 0x8CCF6284;
    //unsigned short *p2_health = (unsigned short *) 0x8CCF7402;
    unsigned char *mem_start = (unsigned char *) 0x8C010000;
    unsigned char *mem_end = (unsigned char *) 0x8CFFFFFF;

    ubc_serial_init(57600);
    ubc_serial_flush();
    ubc_serial_write_str("[UBC] Hit from 0x");
    uint_to_string(spc(), num_buffer);
    ubc_serial_write_str(num_buffer);
    ubc_serial_write_str("\r\n");
    
#if 0

    /* Today we're scanning for the customized VR information */
    if (*p1_health < 500 && *p1_health > 50)
    {
        /* Search for the byte signature: */
        unsigned char prototem_sig[] = {
            0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3,
            0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E,
        };
        unsigned char *cur_loc;
        unsigned long did_find;

        ubc_serial_write_str("[UBC] Starting prototem search\n\r");

        did_find = 0;
        for (cur_loc = mem_start; cur_loc < mem_end; cur_loc++)
        {
            if (*cur_loc == prototem_sig[0])
            {
                if(!memcmp(cur_loc, prototem_sig, sizeof(prototem_sig)))
                {
                    unsigned char *check_loc;
                    char num_buffer[10];

                    /* MATCH */
                    ubc_serial_write_str("[UBC] Prototem match confirmed @ 0x");
                    uint_to_string((unsigned int) cur_loc, num_buffer);
                    ubc_serial_write_str(num_buffer);
                    ubc_serial_write_str("\r\n");

                    ubc_serial_write_str("[UBC] Writing ");
                    check_loc = cur_loc + (3*5);
                    for (; cur_loc <= check_loc; cur_loc++)
                    {
                        ubc_serial_write_str(".");
                        *cur_loc = 0xFF;
                    }
                    ubc_serial_write_str(" done!\r\n");

                    cur_loc += 1024 - (3*5);

                    did_find = (unsigned long) cur_loc;
                }
            }
        }
        ubc_serial_write_str("[UBC] Prototem search complete.\r\n");

#ifdef POINTER_FIND
        if (did_find)
        {
            ubc_serial_write_str("[UBC] Starting pointer search.\r\n");
            /* Now scan for the pointer in memory */
            for (cur_loc = mem_start; cur_loc < mem_end; cur_loc += 4)
            {
                if (*((unsigned long *) cur_loc) == did_find)
                {
                    char num_buffer[10];

                    ubc_serial_write_str("[UBC] Pointer match confirmed @ 0x");
                    uint_to_string((unsigned int) cur_loc, num_buffer);
                    ubc_serial_write_str(num_buffer);
                    ubc_serial_write_str("\r\n");
                }
            }
            ubc_serial_write_str("[UBC] Pointer search complete.\r\n");
        }
#endif
        
        ubc_serial_write_str("[UBC] All searches complete.\r\n");
        *p1_health = 1000;
    }
#endif
}

#define LOADED_POINT        0x8C300000
#define REAL_LOAD_POINT     0x8C010000

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
