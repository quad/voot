/*  client.h

DESCRIPTION

    The console netplay client header file.

CHANGELOG

    Tue Jan 22 17:51:26 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added this changelog. The file has actually been around for quite
        some time.

*/

#ifndef __NPCLIENT_H__
#define __NPCLIENT_H__

#include "vars.h"

/*
 *  Various frontend texts.
 */

char banner_text[] = {
    "Console Netplay VOOT Client (npclient), built " __DATE__ " at " __TIME__ "\n"
    "Copyright (C) 2001, 2002, Scott Robinson. All Rights Reserved.\n"
};

char gpl_text[] = {
    "\nThis program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
};

char help_text[] = {
    "-c <hostname/IP[:port]>    Connect to the specified slave.\n"
    "-s <hostname/IP[:port]>    Connect to the specified server.\n"
    "-l[port]                   Change into server mode.\n"
    "\n"
};

char exit_text[] = {
    "npclient exit!\n"
};

/*
 *  Frontend Prototypes
 */

void display_start_banner(void);
void display_exit_banner(void);

void frontend_init(char *pname);
void frontend_cleanup(void);

void parse_options(int32 argc, char *argv[]);

void* input_poll(void *arg);
void input_handler(char *line);

#endif
