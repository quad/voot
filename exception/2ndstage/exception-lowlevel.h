#ifndef __EXCEPTION_LOWLEVEL_H__
#define __EXCEPTION_LOWLEVEL_H__

#include "vars.h"

/* The UBC Registers */
#define _UBC_BASE       (0xFF200000)
#define _UBC_ASID_BASE  (0xFF000000)

#define UBC_R_BARA      (REGISTER(int)      (_UBC_BASE))
#define UBC_R_BAMRA     (REGISTER(char)     (_UBC_BASE + 0x04))
#define UBC_R_BBRA      (REGISTER(short)    (_UBC_BASE + 0x08))
#define UBC_R_BASRA     (REGISTER(char)     (_UBC_ASID_BASE + 0x14))
#define UBC_R_BRCR      (REGISTER(short)    (_UBC_BASE + 0x20))

extern uint8 my_vbr_table[];
extern uint8 my_vbr_table_end[];

extern uint8 general_sub_handler[];
extern uint8 general_sub_handler_base[];
extern uint8 general_sub_handler_end[];

extern uint8 cache_sub_handler[];
extern uint8 cache_sub_handler_base[];
extern uint8 cache_sub_handler_end[];

extern uint8 interrupt_sub_handler[];
extern uint8 interrupt_sub_handler_base[];
extern uint8 interrupt_sub_handler_end[];

extern void exception_handler_lowlevel(void);
extern void my_exception_finish(void);
extern void ubc_wait(void);

#endif
