/*  loader.c

    The first stage loader of low-memory driver:
    1) Load a game's 1ST_READ.BIN
    2) Load the low-memory driver into memory
    3) Execute the low-memory driver
*/

#include <stdio.h>
#include <dream.h>
#include "vconsole.h"
#include "gdrom.h"
#include "2ndstage.h"

#define FIRST_LOAD_POINT    0x8C300000
#define FIRST_RUN_POINT     0x8C010000
#define FIRST_LOAD_FILE     "/1st_read.bin"

static char *stage_buffer = (char *) 0xAC004000;
static char *first_load_buffer = (char *) FIRST_LOAD_POINT;
static unsigned long *first_load_size = (unsigned long *) FIRST_RUN_POINT;


#define COLOR_FIRST_WAIT    0, 0, 0
#define COLOR_BOOT_INIT     0, 0, 100

static void boot_loader(unsigned char *bootstrap_name)
{
    int fd, bin_size;
    int do_warez;

    vc_clear(COLOR_BOOT_INIT);

    vc_puts("Initializing ...");

    cdrom_reinit();
    iso_init();

    /* Attempt to load the media. If it's a GD-ROM, the bootstrap and second
       stage are loaded into memory and plain executed. If the media is a
       CD, however, we need to descramble the 1ST_READ.BIN in order to use
       it. */
    vc_puts("Accessing ...");
    do_warez = open_gd_or_cd(&fd, bootstrap_name);
    
    if (!fd)
    {
        vc_puts("We are not familiar with the media inserted.\n");
        return;
    }

    /* Load the actual bootstrap into memory */
    bin_size = iso_total(fd);
    /* vc_printf("Loading %s (%ub)...", bootstrap_name, bin_size); */
    vc_puts("Loading ...");
    iso_read(fd, first_load_buffer, bin_size);
    iso_close(fd);

    if (do_warez)
        vc_puts("WAREZ_LOAD active! Bad pirate.");

    /* Copy the second stage into the IP.BIN area and execute it */
    memcpy(stage_buffer, stage_two_bin, stage_two_bin_size);

    vc_puts("Go!");
    *first_load_size = bin_size;
    (*(void (*)()) stage_buffer) (do_warez);    /* If you don't screw with R4. */
}

void dc_main(void)
{
    vid_init(vid_check_cable(), DM_640x480, PM_RGB565);

    vc_clear(COLOR_FIRST_WAIT);

    cdrom_init();

    /* actual character limit 
    vc_puts("--------------------------------------------------");  */
    vc_puts("Netplay VOOT Extensions (np-voot-slave) - BETA");
    vc_puts("(C) 2001-2002, Scott Robinson. All Rights Reserved.");
    vc_puts("http://voot.sourceforge.net/ for more information.");
    vc_puts("");
    vc_puts("This program is distributed in the hope that it");
    vc_puts("will be useful, but WITHOUT ANY WARRANTY; without");
    vc_puts("even the implied warranty of MERCHANTABILITY or");
    vc_puts("FITNESS FOR A PARTICULAR PURPOSE.  See the GNU");
    vc_puts("General Public License for more details.");
    vc_puts("");
    vc_puts("(loader built at " __TIME__" on " __DATE__ ")");
    vc_puts(stage_two_build_time);
    vc_puts("");

    while (1)
    {
        vc_puts("Please insert a Virtual-On GD-ROM.");
        wait_for_gdrom();

        boot_loader(FIRST_LOAD_FILE);

        vc_puts("An error occured accessing your GD-ROM.");
    }
}
