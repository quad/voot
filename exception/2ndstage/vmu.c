/*  vmu.c

DESCRIPTION

    VMU access functions into VOOT's (Katana's?) actual code.

*/

#include "vars.h"
#include "util.h"

#include "vmu.h"

static uint8 *vmu_load_root;
static uint8 *vmu_status_root;
static uint8 *vmu_exists_root;
static uint8 *vmu_mount_root;
static uint32 vmu_mount_buffer;

static const uint8 vmu_load_key[] = { 0xe6, 0x2f, 0xd6, 0x2f, 0xc6, 0x2f, 0x22, 0x4f, 0xf8, 0x7f, 0x9b, 0xd3, 0x43, 0x6e };
static const uint8 vmu_status_key[] = { 0x22, 0x4f, 0x40, 0xd3, 0x0b, 0x43, 0x09, 0x00, 0x08, 0x20 };
static const uint8 vmu_exists_key[] = { 0x22, 0x4f, 0xf8, 0x7f, 0x57, 0xd3 };
static const uint8 vmu_mount_key[] = { 0x22, 0x4f, 0xfc, 0x7f, 0x0e, 0xd3, 0x0b, 0x43, 0x42, 0x2f };


void vmu_init (void)
{
    if (!vmu_load_root)
        vmu_load_root = search_sysmem(vmu_load_key, sizeof(vmu_load_key));

    if (!vmu_status_root)
        vmu_status_root = search_sysmem(vmu_status_key, sizeof(vmu_status_key));

    if (!vmu_exists_root)
        vmu_exists_root = search_sysmem(vmu_exists_key, sizeof(vmu_exists_key));

    if (!vmu_mount_root)
    {
        vmu_mount_root = search_sysmem(vmu_mount_key, sizeof(vmu_mount_key));

        /* STAGE: The extra magic here is that we need to know VOOT's mount
            location. Thus, we search for the single reference to the mount
            function and just backup by one uint32 for the buffer. It's a
            hack, but it works in both US and JP versions of DC-VOOT.
            Hopefully the new DC-VOOT-JP won't be a remaster. */
        vmu_mount_buffer = *( ( (uint32 *) search_sysmem((uint8 *) &vmu_mount_root, sizeof(vmu_mount_root)) ) - 1 )   ;
    }
}

uint32 vmu_load_file (vmu_port port, char *filename, uint8 *obuffer, uint32 num_blocks)
{
    if (vmu_load_root)
        return (*(uint32 (*)()) vmu_load_root)(port, filename, obuffer, num_blocks);
    else
        return VMU_ERR_GENERIC;
}

uint32 vmu_status (vmu_port port)
{
    if (vmu_status_root)
        return (*(uint32 (*)()) vmu_status_root)(port);
    else
        return VMU_STATUS_BUSY;
}

uint32 vmu_exists_file (vmu_port port, char *filename)
{
    if (vmu_exists_root)
        return (*(uint32 (*)()) vmu_exists_root)(port, filename);
    else
        return VMU_ERR_GENERIC;
}

uint32 vmu_mount (vmu_port port)
{
    if (vmu_mount_root)
        return (*(uint32 (*)()) vmu_mount_root)(port, vmu_mount_buffer, 0xf420);
    else
        return VMU_ERR_GENERIC;
}
