/*  biosfont-lowlevel.h

    $Id: biosfont-lowlevel.h,v 1.2 2002/06/24 01:09:41 quad Exp $

*/

#ifndef __COMMON_BIOSFONT_LOWLEVEL_H__
#define __COMMON_BIOSFONT_LOWLEVEL_H__

#define BFONT_GET_ADDRESS   0
#define BFONT_LOCK          1
#define BFONT_UNLOCK        2

/* NOTE: External definitions. */

extern uint32           bfont_bios          (uint32 call);

#define bfont_get_address() REGISTER(uint8) bfont_bios  (BFONT_GET_ADDRESS)
#define bfont_lock()        (int32)         bfont_bios  (BFONT_LOCK)
#define bfont_unlock()      (void)          bfont_bios  (BFONT_UNLOCK)

#endif
