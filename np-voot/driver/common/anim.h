/*  anim.h

    $Id: anim.h,v 1.1 2002/06/29 12:57:04 quad Exp $

*/

#ifndef __COMMON_ANIM_H__
#define __COMMON_ANIM_H__

#include "system.h"

typedef void (* anim_render_chain_f)    (uint16, uint16);
typedef void (* anim_printf_debug_f)    (float, float, uint32, uint32, uint32, uint32, ...);

/* NOTE: External definitions. */

extern  anim_printf_debug_f         __anim_printf_debug;

/* NOTE: Module definitions. */

void    anim_init                   (void);
bool    anim_add_render_chain       (anim_render_chain_f new_function, anim_render_chain_f *old_function);

#define anim_printf_debug(x, y, args...)    (__anim_printf_debug    (x, y, NULL, NULL, NULL, NULL, args))

#endif
