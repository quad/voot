/*  searchmem.h

    $Id: searchmem.h,v 1.1 2002/06/12 10:29:01 quad Exp $

*/

#ifndef __COMMON_SEARCHMEM_H__
#define __COMMON_SEARCHMEM_H__

/* NOTE: Module definitions. */

uint8 * search_sysmem       (const uint8 *key, uint32 key_size);
uint8 * search_sysmem_at    (const uint8 *key, uint32 key_size, const uint8 *start_loc, const uint8 *end_loc);

#endif
