/*  customize.h

    $Id: customize.h,v 1.1 2002/06/12 00:06:08 quad Exp $

*/

#ifndef __DRIVER_CUSTOMIZE_H__
#define __DRIVER_CUSTOMIZE_H__

#include "system.h"

#define CUSTOMIZE_VMU_SIZE          10
#define CUSTOMIZE_VMU_HEAD_IDX      0x282
#define CUSTOMIZE_VMU_VR_IDX        0x2A0
#define CUSTOMIZE_VMU_COLOR_IDX     0x2B0
#define CUSTOMIZE_PALETTE_SIZE      0x200

/* NOTE: This is ARGB. (0x00RRGGBB) */

#define VIDEO_BORDER_COLOR          *((uint32 *) 0xa05f8040)
#define VIDEO_COLOR_BLUE            0x0000ff
#define VIDEO_COLOR_RED             0xff0000
#define VIDEO_COLOR_BLACK           0x000000
#define VIDEO_COLOR_WHITE           0xffffff

typedef enum
{
    LOAD,
    RUN
} customize_check_mode;

typedef enum
{
    CV_NONE,
    CV_HEADS,
    CV_COLORS
} customize_vector_mode;

typedef enum
{
    C_IPC_START,
    C_IPC_MOUNT_SCAN_1,
    C_IPC_MOUNT_SCAN_2,
    C_IPC_LOAD
} customize_ipc;

typedef enum
{
    VR_DORDRAY,
    VR_BALSERIES,
    VR_CYPHER,
    VR_GRYSVOK,
    VR_APHARMDB,
    VR_APHARMDB_B,
    VR_RAIDEN,
    VR_TEMJIN,
    VR_FEIYEN,
    VR_ANGELAN,
    VR_SPECINEFF,
    VR_APHARMDS,
    VR_AJIM,
    VR_SENTINEL
} voot_vr_id;

typedef struct
{
    uint8 head;

    uint8 palette[CUSTOMIZE_PALETTE_SIZE];
} customize_data;

void    customize_init      (void);
void *  customize_handler   (register_stack *stack, void *current_vector);

#endif
