/*  mem.c

DESCRIPTION

    Interface module emulating lwIP's bss space memory manager. This
    translates the functions over to the Katana heap.

TODO

    Finish integrating the debugging statistics functions.

*/

#include <vars.h>
#include <malloc.h>

void mem_init (void)
{
    uint32 freesize, max_freesize;

    malloc_stat (&freesize, &max_freesize);

#ifdef MEM_STATS
    stats.mem.avail = max_freesize;
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
