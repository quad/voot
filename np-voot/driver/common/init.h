/*  init.h

    $Id: init.h,v 1.5 2002/11/24 14:56:45 quad Exp $

*/

#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

extern uint8    bss_start[];
extern uint8    *end;

/* NOTE: Module definitions. */

void    np_initialize       (void *arg1, void *arg2, void *arg3, void *arg4)    __attribute__ ((noreturn));
void    np_configure        (void);
void    np_reconfigure      (void);

#endif
