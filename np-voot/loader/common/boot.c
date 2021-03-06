/*  boot.c

    $Id: boot.c,v 1.2 2002/12/18 00:18:48 quad Exp $

DESCRIPTION

    Module containing all the bootloader functionality.

CHANGELOG

    Tue May 28 09:19:02 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Split out the boot loading functionality from the main module.

    Thu Jun  6 01:31:29 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Miscellaneous code cleanups.

    Mon Jun 24 22:53:47 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Shifted load address to new safe page.

    Wed Nov  6 17:57:34 PST 2002    Scott Robinson <scott_vo@quadhome.com>
        Added the filename option and "merged" the two versions of the same
        bootloader.

    Tue Dec 17 16:04:16 PST 2002    Scott Robinson <scott_vo@quadhome.com>
        Made the caller responsible for specifying the driver load location.
        Added a paranoia if statement to the beginning of the boot_loader
        process.

*/

#include <kos.h>

#include "boot.h"

void boot_loader (const char *filename, uint8 *stage_buffer)
{
    int     in_driver;
    uint8  *driver_buffer;

    if (!filename || !stage_buffer)
        return;

    /* STAGE: Open and read in the driver binary. */

    in_driver = fs_open (filename, O_RDONLY);

    /*
        STAGE: If the driver wasn't found, it means this loader package is
        broken.
    */

    if (!in_driver)
        return;

    /*
        STAGE: Because we only use ROMFS, we can memory map the driver
        binary and then just perform an arch_exec ().
    */

    driver_buffer = fs_mmap (in_driver);

    /* STAGE: Tell KOS to load up the driver. */

    arch_exec_at (driver_buffer, fs_total (in_driver), (uint32) stage_buffer);
}
