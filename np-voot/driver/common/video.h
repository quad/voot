/*  video.h

    $Id: video.h,v 1.1 2002/06/12 09:33:51 quad Exp $

*/

#ifndef __COMMON_VIDEO_H__
#define __COMMON_VIDEO_H__

#define VIDEO_VRAM_START    ((uint16 *) (0xa5000000 + *(REGISTER(uint32) 0xa05f8050)))
#define VIDEO_FB_BUFFER     0xa05f8050

/* NOTE: Module definitions. */

void    video_waitvbl   (void);
void    video_clear     (int16 r, int16 g, int16 b);

#endif
