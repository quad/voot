#ifndef __HUD_H__
#define __HUD_H__

extern bool do_hud;

extern volatile uint16 *vid_mem;

void hud_init(void);
void hud_write_line(const char *in_line);
int32 hud_printf(const char *fmt, ...);
void render_hud(void);
void display_hud(void);

#endif
