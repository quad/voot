/*  init.h

    $Id: init.h,v 1.2 2002/06/20 10:20:05 quad Exp $

*/

#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

extern uint8    bss_start[];
extern uint8    *end;

/* NOTE: Module definitions. */

void    np_initialize       (void *arg1, void *arg2, void *arg3, void *arg4);
void    np_configure        (void);
void    handle_bios_vector  (void);

#endif
