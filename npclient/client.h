/*  client.h

DESCRIPTION

    The console netplay client header file.

CHANGELOG

    Tue Jan 22 17:51:26 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added this changelog. The file has actually been around for quite
        some time.

    Tue Jan 22 23:37:22 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Removed the various texts section. It could potentially cause
        problems in the future.

*/

#ifndef __NPCLIENT_H__
#define __NPCLIENT_H__

#include "vars.h"

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
