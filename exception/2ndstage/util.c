/*  util.c

    Yeah, everyone needs a utility module.
*/

#include "vars.h"
#include "voot.h"
#include "util.h"

/* Borrowed from Dan's libc */
void* memmove(void *dest, const void *src, uint32 count)
{
    char *tmp, *s;

    if (dest <= src)
    {
        tmp = (char *) dest;
        s = (char *) src;

        while (count--)
            *tmp++ = *s++;
    }
    else
    {
        tmp = (char *) dest + count;
        s = (char *) src + count;

        while (count--)
            *--tmp = *--s;
    }

    return dest;
}

/* Borrowed from libdream. */
void vid_waitvbl(void)
{
    volatile uint32 *vbl = ((volatile uint32 *) 0xa05f8000) + 0x010c / sizeof(uint32);

    while (!(*vbl & 0x01ff));
    while (*vbl & 0x01ff);
}

/* Moved from the original search module. */
uint8* search_sysmem(const uint8 *key, uint32 key_size)
{
    return search_sysmem_at(key, key_size, SYS_MEM_START, SYS_MEM_END);
}

uint8* search_sysmem_at(const uint8 *key, uint32 key_size, uint8 *start_loc, uint8 *end_loc)
{
    uint8 *cur_loc;

    for (cur_loc = start_loc; cur_loc < end_loc; cur_loc++)
        if (*cur_loc == key[0])
            if(!memcmp((const uint8 *) cur_loc, key, key_size))
                return cur_loc;     /* So we have a match. Report it. */

    return 0x0;     // I really should define NULL
}

void grep_memory(const char *string)
{
    uint8 *mem_loc;

    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Grepping memory for '%s' ...\n", string);

    mem_loc = SYS_MEM_START;

    while ((mem_loc = search_sysmem_at(string, strlen(string), mem_loc, SYS_MEM_END)))
    {
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Match @ %x\n", mem_loc);
        mem_loc++;
    }

    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Grep done!\n");
}

/* Stolen from VOOT! Accessor to syMalloc() */
uint8 *malloc_root;
const uint8 malloc_key[] = { 0xE6, 0x2F, 0xFC, 0x7F, 0x02, 0x00 };

void malloc_init(void)
{
    malloc_root = search_sysmem(malloc_key, sizeof(malloc_key));
}

void* malloc(uint32 size)
{
    if (malloc_root)
        return (*(void* (*)()) malloc_root)(size);
    else
        return 0x0;
}

void free(void *data)
{
    if (malloc_root)
        return (*(void (*)()) (malloc_root + MALLOC_FREE_INDEX))(data);
}
