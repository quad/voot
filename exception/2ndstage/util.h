#ifndef __UTIL_H__
#define __UTIL_H__

#include "vars.h"

#define SYS_MEM_START       ((uint8 *) 0x8C010000)
#define GAME_MEM_START      ((uint8 *) 0x8C270000)
#define SYS_MEM_END         ((uint8 *) 0x8CFFFFFF)

#define MALLOC_FREE_INDEX   0xCE

#define tolower(c)      ((c)-'A'+'a')
#define toupper(c)      ((c)-'a'+'A')
#define isdigit(c)	    ((c) >= '0' && (c) <= '9')
#define isxdigit(c)	    (isdigit(c) || ((toupper(c)>='A') && (toupper(c)<='F')))
#define islower(c)      ((c) >= 'a' && (c) <= 'z')
#define strnlen(s, max) ((strlen(s) > max) ? max : strlen(s))

#define SAFE_UINT32_COPY(trgt, src) {                           \
    *(((uint16 *) &(trgt))    ) = *(((uint16 *) &(src))    );   \
    *(((uint16 *) &(trgt)) + 1) = *(((uint16 *) &(src)) + 1);   \
                                    }

typedef union unikey
{
    uint32  i;
    uint8   c[sizeof(uint8 *)];
} unikey;

extern uint32 malloc_fail_count;

/* Standard Library functions. We don't want to import the actual headers
   themselves! */

void* memcpy(void *dest, const void *src, uint32 n);
int memcmp(const void *s1, const void *s2, uint32 n);
void *memset(void *s, int32 c, uint32 n);
uint32 strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strncpy(char *dest, const char *src, uint32 n);

/* Our prototypes. */

void* memmove(void *dest, const void *src, uint32 count);
void vid_waitvbl(void);
uint8* search_sysmem(const uint8 *key, uint32 key_size);
uint8* search_sysmem_at(const uint8 *key, uint32 key_size, const uint8 *start_loc, const uint8 *end_loc);
void grep_memory(const uint8 *key, uint32 key_size);
void malloc_init(void);
void* malloc(uint32 size);
void free(void *data);
void* sq_cpy(void *dest, const uint32 *src, uint32 n);
uint32 time(void);

#endif
