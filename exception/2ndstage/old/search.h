#ifndef __SEARCH_H__
#define __SEARCH_H__

#define SYS_MEM_START   ((unsigned char *) 0x8C010000)
#define SYS_MEM_END     ((unsigned char *) 0x8CFFFFFF)

typedef union unikey
{
    uint32  i;
    uint8   c[sizeof(uint8 *)];
} unikey;

unsigned char* search_sysmen(unsigned char *key, unsigned int key_size);
unsigned char* search_sysmem_at(unsigned char *key, unsigned int key_size, unsigned char *start_loc, unsigned char *end_loc);

#endif
