/*  malloc.h

    $Id: malloc.h,v 1.3 2002/11/08 19:47:41 quad Exp $

*/

#ifndef __COMMON_MALLOC_H__
#define __COMMON_MALLOC_H__

#define MALLOC_MALLOC_INDEX     0x84
#define MALLOC_FREE_INDEX       0x152
#define MALLOC_REALLOC_INDEX    0x20a

/* NOTE: Module definitions. */

void    malloc_init (void);
void    malloc_stat (uint32 *freesize, uint32 *max_freesize);
void *  malloc      (uint32 size);
void    free        (void *data);
void *  realloc     (void *data, uint32 size);

#endif
