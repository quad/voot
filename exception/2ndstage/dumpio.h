#ifndef __DUMPIO_H__
#define __DUMPIO_H__

typedef struct
{
    uint32 target;
    uint32 index;
} dump_control_t;

void dump_framebuffer(void);
void dump_add(const uint8 *in_data, uint32 in_data_size);
void dump_start(uint32 target_loc);
uint32 dump_stop(void);

#endif
