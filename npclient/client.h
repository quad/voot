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

    Tue Feb 26 15:50:30 PST 2002    Scott Robinson <scott_vo@quadhome.com>
        Removed a lot of the prototypes, because they're now properly
        "static" in the module. However, I added the callback function and
        threaded prototypes.

*/

#ifndef __NPCLIENT_H__
#define __NPCLIENT_H__

#include "vars.h"

/*
 *  Callback Prototypes
 */


void* input_poll(void *arg);
void input_handler(char *line);

void logger_callback(npc_log_level severity, const char *format, ...);
bool packet_callback(uint8 type, const voot_packet *packet);

#endif
