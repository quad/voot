/*  debug.h

    $Id: debug.h,v 1.1 2002/06/12 00:35:40 quad Exp $

*/

#ifndef __LOADER_DEBUG_H__
#define __LOADER_DEBUG_H__

static char startup_msg[] = {
    "Debugging Loader (np-voot-loader-debug) - ALPHA\n"
    "(C) 2002, the VO Reverse Engineering Project.\n"
    "All Rights Reserved.\n"
    "Go to http://voot.sf.net/ for more information.\n"
    "\n"
    "This program is distributed in the hope that it\n"
    "will be useful, but WITHOUT ANY WARRANTY;\n"
    "without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR\n"
    "PURPOSE. See the GNU General Public License for\n"
    "more details.\n"
    "\n"
    "(loader built at " __TIME__" on " __DATE__ ")\n"
};

static char insert_disc_msg[] = {
    "\n"
    "Please insert a Virtual-On GDROM.\n"
    "\n"
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

#endif
