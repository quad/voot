/*  mem.c

    $Id: mem.c,v 1.4 2002/12/17 11:55:01 quad Exp $

DESCRIPTION

    Interface module emulating lwIP's bss space memory manager. This
    translates the functions over to the Katana heap.

*/

#include <vars.h>
#include <malloc.h>

#include "lwip/arch.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "lwip/stats.h"

void mem_init (void)
{
    malloc_init ();

#ifdef MEM_STATS
    malloc_stat (&stats.mem.max, &stats.mem.avail);
#endif /* MEM_STATS */
}

void* mem_malloc (mem_size_t size)
{
    return malloc (size);
}

void mem_free (void *mem)
{
    return free (mem);
}

void* mem_realloc (void *mem, mem_size_t size)
{
    return realloc (mem, size);
}

void* mem_reallocm (void *rmem, mem_size_t newsize)
{
    void   *nmem;

    nmem = malloc (newsize);

    if(nmem == NULL)
    {
        return realloc (rmem, newsize);
    }

    bcopy (rmem, nmem, newsize);
    free (rmem);

    return nmem;
}
