#ifndef __CUSTOMIZE_H__
#define __CUSTOMIZE_H__

#include "system.h"

typedef enum
{
    LOAD,
    RUN
} customize_check_mode;

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
    VR_AJIM             /* 12 */
} voot_vr_id;

typedef struct
{
    uint8 palette[0x200];
} customize_data;

void customize_init(void);
bool customize_reinit(void);
void customize_clear_player(uint32 side);
void* customize_handler(register_stack *stack, void *current_vector);
void maybe_load_customize();

#endif
