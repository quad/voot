/*  malloc.c

    $Id: malloc.c,v 1.1 2002/06/12 10:29:01 quad Exp $

DESCRIPTION

    64k of dynamic memory allocation, provided by VOOT.

*/

#include "vars.h"
#include "util.h"
#include "searchmem.h"

#include "malloc.h"

static uint8       *malloc_root;
static const uint8  malloc_key[] = { 0xe6, 0x2f, 0xc6, 0x2f, 0xfc, 0x7f, 0x02, 0x00 };

void malloc_init (void)
{
    if (!malloc_root || memcmp (malloc_root, malloc_key, sizeof (malloc_key)))
        malloc_root = search_sysmem (malloc_key, sizeof (malloc_key));
}

void malloc_stat (uint32 *freesize, uint32 *max_freesize)
{
    if (malloc_root)
        return (*(void (*)()) malloc_root) (freesize, max_freesize);
}

void* malloc (uint32 size)
{
    void *mem;

    if (malloc_root)
    {
        mem = (*(void* (*)()) (malloc_root + MALLOC_MALLOC_INDEX)) (size);

        return mem;
    }
    else
    {
        return NULL;
    }
}

void free (void *data)
{
    if (malloc_root)
        return (*(void (*)()) (malloc_root + MALLOC_FREE_INDEX)) (data);
}
