#ifndef __BIOSFONT_H__
#define __BIOSFONT_H__

#define BFONT_CHAR_WIDTH    12
#define BFONT_CHAR_HEIGHT   24

void bfont_init(void);
bool bfont_draw(uint16 *buffer, uint32 bufwidth, uint32 c);
bool bfont_draw_str(uint16 *buffer, uint32 width, const char *str);

#endif
