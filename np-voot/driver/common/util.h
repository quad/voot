/*  util.h

    $Id: util.h,v 1.4 2002/06/20 10:20:05 quad Exp $

*/

#ifndef __COMMON_UTIL_H__
#define __COMMON_UTIL_H__

#include "vars.h"

#define MALLOC_MALLOC_INDEX 0x84
#define MALLOC_FREE_INDEX   0x152

#define tolower(c)          ((c)-'A'+'a')
#define toupper(c)          ((c)-'a'+'A')
#define isdigit(c)	        ((c) >= '0' && (c) <= '9')
#define isxdigit(c)	        (isdigit(c) || ((toupper(c)>='A') && (toupper(c)<='F')))
#define islower(c)          ((c) >= 'a' && (c) <= 'z')
#define strnlen(s, max)     ((strlen(s) > max) ? max : strlen(s))

#define SAFE_UINT32_COPY(trgt, src) {                       \
    *(((uint8 *) &(trgt))    ) = *(((uint8 *) &(src))    ); \
    *(((uint8 *) &(trgt)) + 1) = *(((uint8 *) &(src)) + 1); \
    *(((uint8 *) &(trgt)) + 2) = *(((uint8 *) &(src)) + 2); \
    *(((uint8 *) &(trgt)) + 3) = *(((uint8 *) &(src)) + 3); \
                                    }
/*
    NOTE: Standard Library functions.

    We don't want to import the actual headers themselves!
*/

void *  memcpy  (void *dest, const void *src, uint32 n);
int     memcmp  (const void *s1, const void *s2, uint32 n);
void *  memset  (void *s, int32 c, uint32 n);
uint32  strlen  (const char *s);
int     strcmp  (const char *s1, const char *s2);
char *  strncpy (char *dest, const char *src, uint32 n);

/* NOTE: Module defintions. */

void  * memmove             (void *dest, const void *src, uint32 count);
void *  sq_cpy              (void *dest, const uint32 *src, uint32 n);
uint32  time                (void);

#endif
