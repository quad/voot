/*  searchmem.h

    $Id: searchmem.h,v 1.2 2002/06/29 12:57:04 quad Exp $

*/

#ifndef __COMMON_SEARCHMEM_H__
#define __COMMON_SEARCHMEM_H__

/* NOTE: Module definitions. */

void *  search_sysmem       (const uint8 *key, uint32 key_size);
void *  search_gamemem      (const uint8 *key, uint32 key_size);
void *  search_memory_at    (const uint8 *key, uint32 key_size, const uint8 *start_loc, const uint8 *end_loc);

#endif
