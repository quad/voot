/*  boot.c

    $Id: boot.c,v 1.2 2002/06/29 13:02:27 quad Exp $

DESCRIPTION

    Module containing all the bootloader functionality.

CHANGELOG

    Tue May 28 09:19:02 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Split out the boot loading functionality from the main module.

    Thu Jun  6 01:31:29 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Miscellaneous code cleanups.

    Mon Jun 24 22:53:47 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Shifted load address to new safe page.

*/

#include <kos.h>

#include "boot.h"

void boot_loader (void)
{
    int     in_driver;
    uint8  *driver_buffer;
    uint8  *stage_buffer;
    
    stage_buffer = (uint8 *) 0x8c250000;

    /* STAGE: Open and read in the driver binary. */

    in_driver = fs_open ("/rd/driver.bin", O_RDONLY);

    /*
        STAGE: If the driver wasn't found, it means this loader package is
        broken.
    */

    if (!in_driver)
        return;

    /*
        STAGE: Because we only use ROMFS, we can memory map the driver
        binary and then just perform a normal memcpy.
    */

    driver_buffer = fs_mmap (in_driver);

    /* STAGE: Tell KOS to load up the driver. */

    arch_exec_select (driver_buffer, fs_total (in_driver), (uint32) stage_buffer);
}
