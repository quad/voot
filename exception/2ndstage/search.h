#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "vars.h"

#define SYS_MEM_START   ((volatile uint8 *) 0x8C010000)
#define SYS_MEM_END     ((volatile uint8 *) 0x8CFFFFFF)

typedef union unikey
{
    uint32  i;
    uint8   c[sizeof(uint8 *)];
} unikey;

volatile uint8* search_sysmem(const uint8 *key, uint32 key_size);
volatile uint8* search_sysmem_at(const uint8 *key, uint32 key_size, volatile uint8 *start_loc, volatile uint8 *end_loc);
void search_grep_memory(const char *string);

#endif
