/*  module.h

    $Id: module.h,v 1.2 2002/06/12 04:41:34 quad Exp $

*/

#ifndef __DRIVER_MODULE_H__
#define __DRIVER_MODULE_H__

void    module_initialize   (void);
void    module_configure    (void);
void    module_bios_vector  (void);

void *  debug_handler       (register_stack *stack, void *current_vector);

#endif
