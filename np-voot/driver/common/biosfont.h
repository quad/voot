/*  biosfont.h

    $Id: biosfont.h,v 1.3 2002/06/24 00:58:04 quad Exp $

*/

#ifndef __COMMON_BIOSFONT_H__
#define __COMMON_BIOSFONT_H__

#define BFONT_CHAR_WIDTH    12
#define BFONT_CHAR_HEIGHT   24

/* NOTE: Module definitions. */

void    bfont_init      (void);
bool    bfont_draw      (uint16 *buffer, uint32 bufwidth, uint32 c);
bool    bfont_draw_str  (uint16 *buffer, uint32 width, const char *str);

#endif
