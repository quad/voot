/*  biosfont.h

    $Id: biosfont.h,v 1.1 2002/06/11 23:22:51 quad Exp $

*/

#ifndef __COMMON_BIOSFONT_H__
#define __COMMON_BIOSFONT_H__

#define BFONT_CHAR_WIDTH    12
#define BFONT_CHAR_HEIGHT   24

/* NOTE: External definitions */
extern volatile uint8 * bfont_get_address   (void);
extern int32            bfont_lock          (void);
extern void             bfont_unlock        (void);

/* NOTE: Module definitions */

void    bfont_init      (void);
bool    bfont_draw      (uint16 *buffer, uint32 bufwidth, uint32 c);
bool    bfont_draw_str  (uint16 *buffer, uint32 width, const char *str);

#endif
