#ifndef __BIOSFONT_H__
#define __BIOSFONT_H__

#define BFONT_CHAR_WIDTH    12
#define BFONT_CHAR_HEIGHT   24

void bfont_init(void);
volatile uint8* bfont_find_char(uint32 ch);
void bfont_draw(uint8 *buffer, uint32 bufwidth, uint32 c);
void bfont_draw_str(uint8 *buffer, uint32 width, char *str);

#endif
