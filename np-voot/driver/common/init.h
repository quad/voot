/*  init.h

    $Id: init.h,v 1.6 2002/12/16 07:50:56 quad Exp $

*/

#ifndef __COMMON_INIT_H__
#define __COMMON_INIT_H__

typedef void    (* np_reconf_handler_f) ();

/* NOTE: Module definitions. */

void                np_initialize               (void *arg1, void *arg2, void *arg3, void *arg4)    __attribute__ ((noreturn));
void                np_configure                (void);
void                np_reconfigure              (void);
np_reconf_handler_f np_add_reconfigure_chain    (np_reconf_handler_f function);

#endif
