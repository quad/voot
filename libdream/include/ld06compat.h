#ifndef __LD06COMPAT_H
#define __LD06COMPAT_H

/* This is a name compatability header file with libdream 0.6 so you don't
   have as much work to do in porting code. I don't recommend its permanent
   usage though. To use it, include it after dream.h in the source file
   that needs conversion. This isn't a holy grail; I'm afraid that
   pretty much all maple stuff will break and it's just not compatible
   enough to wrap. */

#warning This source file uses ld06compat.h; I recommend switching over

/* libdream.c */
#define dc_sleep sleep

/* video.c */
#define dc_vid_check_cable vid_check_cable
#define dc_vid_init(CT, PM) vid_init(CT, DM_640x480, PM)
#define dc_vid_border_color vid_border_color
#define dc_vid_clear vid_clear
#define dc_vid_waitvbl vid_waitvbl

/* serial.c */
#define dc_serial_init serial_init(57600)
#define dc_serial_printf printf

/* font.c */
#define dc_font_set font_set
#define dc_draw_char draw_char
#define dc_draw_char_2 draw_char_2
#define dc_draw_char_2 draw_char_4
#define dc_draw_string draw_string
#define dc_draw_string_2 draw_string_2
#define dc_draw_string_4 draw_string_4
#define dc_draw_stringf draw_stringf
#define dc_draw_stringf_2 draw_stringf_2
#define dc_draw_stringf_4 draw_stringf_4

/* spu.c */
#define dc_snd_load snd_load
#define dc_snd_load_arm snd_load_arm
#define dc_snd_stop_arm snd_stop_arm
#define dc_snd_init snd_init

/* deprecated from spu.c */
#define dc_snd_play(a, b, c, d, e, f, g, h, i)
#define dc_snd_stop(a)
#define dc_snd_vol(a)
#define dc_snd_pan(a)
#define dc_snd_freq(a)

/* cdfs.c -- these aren't even remotely close anymore */
/* #define dc_gd_open cd_open
#define dc_gd_close cd_close
#define dc_gd_pread cd_pread
#define dc_gd_read cd_read
#define dc_gd_lseek cd_lseek
#define dc_gd_tell cd_tell
#define dc_gd_opendir cd_opendir
#define dc_gd_closedir cd_closedir
#define dc_gd_readdir_r cd_readdir_r
#define dc_gd_readdir cd_readdir
#define dc_gd_init cd_init */



#endif /* __LD06COMPAT_H */

