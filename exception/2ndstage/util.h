#ifndef __UTIL_H__
#define __UTIL_H__

#include "vars.h"

#define SYS_MEM_START   ((uint8 *) 0x8C010000)
#define SYS_MEM_END     ((uint8 *) 0x8CFFFFFF)

#define MALLOC_FREE_INDEX   0xCE

typedef union unikey
{
    uint32  i;
    uint8   c[sizeof(uint8 *)];
} unikey;

void* memmove(void *dest, const void *src, uint32 count);
void vid_waitvbl(void);
uint8* search_sysmem(const uint8 *key, uint32 key_size);
uint8* search_sysmem_at(const uint8 *key, uint32 key_size, uint8 *start_loc, uint8 *end_loc);
void grep_memory(const char *key, uint32 key_size);
void malloc_init(void);
void* malloc(uint32 size);
void free(void *data);

#endif
