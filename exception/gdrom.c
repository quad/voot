/*  gdrom.c

    This code is ripped fairly wholesale from libdream which pretty much
    ripped it wholesale from Marcus' original example code.

    Isn't it good to see code sharing taking place in the Dreamcast
    community?
*/

#include <dream.h>

/* GD-Rom BIOS calls... named mostly after Marcus' code. None have more
   than two parameters; R7 (fourth parameter) needs to describe
   which syscall we want. */

#define MAKE_SYSCALL(rs, p1, p2, idx) \
	uint32 *syscall_bc = (uint32*)0x8c0000bc; \
	int (*syscall)() = (int (*)())(*syscall_bc); \
	rs syscall((p1), (p2), 0, (idx));

/* Check drive status and get disc type */
int gdc_get_drv_stat(void *param)
{
    MAKE_SYSCALL(return, param, 0, 4);
}

/* Uses the GD-ROM sys-calls and checks if we actually have a GD in the
   drive. I don't think this knows the difference between a CD or a GD,
   though. */
void wait_for_gdrom(void)
{
    unsigned int params[4];

    cdrom_reinit();

    /* WAIT ONE:
        Wait for the drive to go empty.
    */

    params[0] = params[1] = 255;
    while (1)
    {
        gdc_get_drv_stat(params);
        if (params[0] == 6)         /* 6 == Drive lid is open */
            break;

        vid_waitvbl();
    }

    /* WAIT TWO:
        Wait for a media we support.
    */

    params[0] = params[1] = 255;
    while(1)
    {
        gdc_get_drv_stat(params);
        if (params[0] == 1 && (params[1] == 0x80 || params[1] == 0x20 || params[1] == 0x10))
            break;

        vid_waitvbl();
    }
}

unsigned int gdrom_disc_type(void)
{
    unsigned int params[4];

    params[0] = params[1] = 255;
    while (1)
    {
        gdc_get_drv_stat(params);
        if (params[0] <= 4)
            break;

        vid_waitvbl();
    }

    return params[1];
}
