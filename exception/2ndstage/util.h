#ifndef __UTIL_H__
#define __UTIL_H__

#include "vars.h"

void uint_to_string(uint32 foo, uint8 *bar);
void* memmove(void *dest, const void *src, uint32 count);
void vid_waitvbl(void);

#endif
