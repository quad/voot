#ifndef __CUSTOMIZE_H__
#define __CUSTOMIZE_H__

#include "system.h"

typedef enum
{
    LOAD,
    RUN
} customize_check_mode;

typedef struct
{
    uint8 vr_type;

    uint8 data[0x40F];
} customize_data;

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

void customize_init(void);
bool customize_reinit(void);
void* customize_handler(register_stack *stack, void *current_vector);
void maybe_load_customize();

#endif
