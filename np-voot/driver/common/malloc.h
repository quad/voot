/*  malloc.h

    $Id: malloc.h,v 1.1 2002/06/12 10:29:01 quad Exp $

*/

#ifndef __COMMON_MALLOC_H__
#define __COMMON_MALLOC_H__

/* NOTE: Module definitions. */

void    malloc_init (void);
void    malloc_stat (uint32 *freesize, uint32 *max_freesize);
void *  malloc      (uint32 size);
void    free        (void *data);

#endif
