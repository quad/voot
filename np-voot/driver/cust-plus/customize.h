/*  customize.h

    $Id: customize.h,v 1.3 2002/10/28 01:13:01 quad Exp $

*/

#ifndef __DRIVER_CUSTOMIZE_H__
#define __DRIVER_CUSTOMIZE_H__

#include "system.h"

#define CUSTOMIZE_VMU_SIZE          10
#define CUSTOMIZE_VMU_HEAD_IDX      0x282
#define CUSTOMIZE_VMU_VR_IDX        0x2A0
#define CUSTOMIZE_VMU_COLOR_IDX     0x2B0
#define CUSTOMIZE_PALETTE_SIZE      0x200

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

#endif
