/*  debug.h

    $Id: debug.h,v 1.2 2002/10/25 20:56:29 quad Exp $

*/

#ifndef __LOADER_DEBUG_H__
#define __LOADER_DEBUG_H__

static char next_page_msg[] = {
    "\n"
    "[Press the A Button to continue...]\n"
};

static char startup_msg[] = {
    "Debugging Loader (np-voot-loader-debug) - ALPHA\n"
    "(loader built at " __TIME__" on " __DATE__ ")\n"
};

static char insert_disc_msg[] = {
    "Please insert a Virtual-On GDROM.\n"
};

static char bad_disc_msg[] = {
    "Unable to access the inserted disc.\n"
    "\n"
    "This is due either to the disc being\n"
    "unreadable or because the disc is not a\n"
    "Virtual-On GD-ROM.\n"
};

static char broken_dist_msg[] = {
    "The loader was unable to load the required\n"
    "drivers for this distribution. Either the\n"
    "distribution is corrupted, or something very\n"
    "mysterious occured during the load process.\n"
    "\n"
    "Unfortunately, it's broken. Sorry!\n"
};

static char no_desc_msg[] = {
    "The loader was unable to load the description\n"
    "of the driver included in this distribution.\n"
    "Either the distribution is corrupted, or\n"
    "something very mysterious occured during the\n"
    "load process.\n"
};

#endif
