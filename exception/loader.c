/*  loader.c

DESCRIPTION

    The first stage loader of low-memory driver:

    1) Load a game's 1ST_READ.BIN
    2) Load the low-memory driver into memory
    3) Execute the low-memory driver

TODO

    Parse the IP.BIN and load the appropriate game binary instead of hard
    coding '1ST_READ.BIN'

*/

#include <stdio.h>
#include <dream.h>
#include "vconsole.h"
#include "gdrom.h"
#include "2ndstage.h"

#define FIRST_LOAD_POINT    0x8C300000
#define FIRST_RUN_POINT     0x8C010000

#define FIRST_LOAD_FILE     "/1st_read.bin"

static char *stage_buffer = (char *) 0x8C004000;
static char *first_load_buffer = (char *) FIRST_LOAD_POINT;
static unsigned long *first_load_size = (unsigned long *) FIRST_RUN_POINT;
static int warez_enable = 1;

static char startup_msg[] = {
    "Netplay VOOT Extensions (np-voot-slave) - BETA\n"
    "(C) 2001-2002, Scott Robinson. All Rights Reserved.\n"
    "http://voot.sourceforge.net/ for more information.\n"
    "\n"
    "This program is distributed in the hope that it\n"
    "will be useful, but WITHOUT ANY WARRANTY; without\n"
    "even the implied warranty of MERCHANTABILITY or\n"
    "FITNESS FOR A PARTICULAR PURPOSE. See the GNU\n"
    "General Public License for more details.\n"
    "\n"
    "(loader built at " __TIME__" on " __DATE__ ")"
};

static char bad_disc_msg[] = {
    "Unable to access the inserted disc.\n"
    "\n"
    "This is due either to the disc being unreadable or\n"
    "because the disc is not a Virtual-On GD-ROM."
};

#define COLOR_FIRST_WAIT    0, 0, 0
#define COLOR_BOOT_INIT     0, 0, 100
#define COLOR_ERROR         100, 0, 0

static void boot_loader(unsigned char *bootstrap_name)
{
    int fd, bin_size;
    int do_warez;

    vc_clear(COLOR_BOOT_INIT);

    vc_puts("Initializing ...");

    cdrom_reinit();
    iso_init();

    vc_puts("Accessing ...");
    do_warez = open_gd_or_cd(&fd, bootstrap_name);
    
    if (!fd)
    {
        vc_clear(COLOR_ERROR);

        vc_puts(bad_disc_msg);

        return;
    }

    /* Load the actual bootstrap into memory */
    bin_size = iso_total(fd);
    vc_puts("Loading ...");
    iso_read(fd, first_load_buffer, bin_size);
    iso_close(fd);

    if (do_warez)
        vc_puts("WAREZ_LOAD active! Bad pirate.");

    /* Copy the second stage into the IP.BIN area and execute it */
    memcpy(stage_buffer, stage_two_bin, stage_two_bin_size);

    vc_puts("Go!");
    *first_load_size = bin_size;
    (*(void (*)()) stage_buffer) (do_warez);
}

void dc_main(void)
{
    /* STAGE: Restart the video system and give us a workable screen. */
    vid_init(vid_check_cable(), DM_640x480, PM_RGB565);
    vc_clear(COLOR_FIRST_WAIT);

    /* STAGE: Initialize the CD-ROM system. We do this again later for paranoia sake. */
    cdrom_init();

    /* STAGE: Display our welcome and copyright text. */ 
    vc_puts(startup_msg);
    vc_puts(stage_two_build_time);

    for (;;)
    {
        vc_puts("\nPlease insert a Virtual-On GD-ROM.");
        wait_for_gdrom();

        if (!warez_enable && (gdrom_disc_type() != GD_TYPE_GDROM))
        {
            vc_clear(COLOR_ERROR);

            vc_puts("The media inserted is not a GD-ROM.");
        }
        else
            boot_loader(FIRST_LOAD_FILE);
    }
}
