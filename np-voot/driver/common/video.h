/*  video.h

    $Id: video.h,v 1.2 2002/08/04 05:48:04 quad Exp $

*/

#ifndef __COMMON_VIDEO_H__
#define __COMMON_VIDEO_H__

#define VIDEO_VRAM_START    ((uint16 *) (0xa5000000 + *(REGISTER(uint32) 0xa05f8050)))
#define VIDEO_FB_BUFFER     0xa05f8050

/* NOTE: This is ARGB. (0x00RRGGBB) */

#define VIDEO_BORDER_COLOR          *((uint32 *) 0xa05f8040)
#define VIDEO_COLOR_BLUE            0x0000ff
#define VIDEO_COLOR_RED             0xff0000
#define VIDEO_COLOR_BLACK           0x000000
#define VIDEO_COLOR_WHITE           0xffffff

/* NOTE: Module definitions. */

void    video_waitvbl   (void);
void    video_clear     (int16 r, int16 g, int16 b);

#endif
