/*  biosfont-lowlevel.h

    $Id: biosfont-lowlevel.h,v 1.1 2002/06/24 00:58:04 quad Exp $

*/

#ifndef __COMMON_BIOSFONT_LOWLEVEL_H__
#define __COMMON_BIOSFONT_LOWLEVEL_H__

/* NOTE: External definitions. */

extern volatile uint8 * bfont_get_address   (void);
extern int32            bfont_lock          (void);
extern void             bfont_unlock        (void);

#endif
