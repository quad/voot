/*  init.h

    $Id: init.h,v 1.1 2002/06/11 20:38:05 quad Exp $

*/

#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

#include "callbacks.h"

extern uint8    bss_start[];
extern uint8    *end;

/* NOTE: Module definitions */

void    np_initialize       (void *arg1, void *arg2, void *arg3, void *arg4);
void    np_configure        (void);
void    handle_bios_vector  (void);

#endif
