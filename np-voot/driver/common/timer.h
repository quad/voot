/*  timer.h

    $Id: timer.h,v 1.1 2002/11/12 08:52:33 quad Exp $

*/

#ifndef __COMMON_TIMER_H__
#define __COMMON_TIMER_H__

#define TIMER_CLOCK_P4      0
#define TIMER_CLOCK_P16     1
#define TIMER_CLOCK_P64     2
#define TIMER_CLOCK_P256    3
#define TIMER_CLOCK_P1024   4

#define TIMER_CLOCK_DEFAULT TIMER_CLOCK_P256

#define TIMER_FREE_COUNT_INDEX      0x026
#define TIMER_FREE_MICRO_INDEX      0x036
#define TIMER_GEN_START_INDEX       0x1a4
#define TIMER_GEN_SET_CLOCK_INDEX   0x274
#define TIMER_GEN_SET_COUNT_INDEX   0x2ae
#define TIMER_GEN_COUNT_INDEX       0x2c8
#define TIMER_GEN_MICRO_INDEX       0x2d2

/* NOTE: Module definitions. */

void    timer_init          (void);
uint32  timer_free_count    (void);
uint32  timer_free_micro    (uint32 count);
void    timer_gen_start     (void);
void    timer_gen_set_clock (uint16 clock);
void    timer_gen_set_count (uint32 count);
uint32  timer_gen_count     (void);
uint32  timer_gen_micro     (uint32 count);

#endif
