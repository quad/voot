#ifndef __NPCLIENT_H__
#define __NPCLIENT_H__

#include "vars.h"

/*
 *  Various texts.
 */

char banner_text[] = {
    "Console Netplay VOOT Client (npclient), built " __DATE__ " at " __TIME__ "\n"
    "Copyright (C) 2001, Scott Robinson. All Rights Reserved.\n"
};

char gpl_text[] = {
    "\nThis program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
};

uint8 help_text[] = {
    "-c [hostname/IP]       Connect to the specified slave.\n"
    "-s [hostname/IP]       Connect to the specified server.\n"
    "-l                     Change into server mode.\n"
    "\n"
};

char exit_text[] = {
    "npclient exit!\n"
};

/*
 *  Prototypes
 */

void display_start_banner(void);
void display_exit_banner(void);

void frontend_init(char *pname);
void frontend_cleanup(void);

void parse_options(int32 argc, char *argv[]);

#endif
