/*  ubc.c

    Initialize the UBC for breakpointing and the 1st_read.bin in memory.
*/

#include <string.h>
#include <dream.h>
#include "ubc.h"
#include "ubc-lowlevel.h"
#include "serial.h"
#include "regs.h"

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

#define BREAKPOINT  ((unsigned int *) (0xFFE8000C))

void dump_registers(unsigned int *reg_pointer, char *hdr, unsigned int cnt)
{
    unsigned int reg_count;
    char buffer[10];

    for (reg_count = 0; reg_count <= cnt; reg_count++)
    {
        ubc_serial_write_str(hdr);

        uint_to_string(reg_count, buffer);
        ubc_serial_write_str(buffer);

        ubc_serial_write_str(" == 0x");

        /* !!! WHY THE HELL DOES THIS WORK?! */        
        uint_to_string(*(reg_pointer - reg_count), buffer);
        ubc_serial_write_str(buffer);

        ubc_serial_write_str(";\r\n");
    }
}

void ubc_break_work(register_stack *stack)
{
    ubc_serial_init(57600);

    ubc_serial_flush();

/*
    dump_registers(&(stack->r0), "R", 14);
    dump_registers(&(stack->r0_bank), "RB", 7);
*/
    ubc_serial_write('#');
    ubc_serial_write(stack->r2);
    
    ubc_serial_flush();
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
    *UBC_R_BRCR = (0<<10) | (1<<15) | 1;

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
