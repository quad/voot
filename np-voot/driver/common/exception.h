/*  exception.h 

    $Id: exception.h,v 1.6 2002/07/06 14:18:15 quad Exp $

*/

#ifndef __COMMON_EXCEPTION_H__
#define __COMMON_EXCEPTION_H__

#include "vars.h"
#include "system.h"
#include "exception-lowlevel.h"

#define EXP_TABLE_SIZE  5

typedef void * (* exception_handler_f)  (register_stack *, void *);

typedef struct
{
    uint32              type;
    uint32              code;

    exception_handler_f handler;
} exception_table_entry;

typedef struct
{
    /* NOTE: Exception counters. */

    uint32      general_exception_count;
    uint32      cache_exception_count;
    uint32      interrupt_exception_count;
    uint32      ubc_exception_count;
    uint32      odd_exception_count;

    /* NOTE: Function hooks for various interrupts. */

    exception_table_entry   table[EXP_TABLE_SIZE];

    /* NOTE: Private status information. */

    bool        vbr_switched;
} exception_table;

typedef enum
{
    FAIL,
    INIT,
    REINIT
} exception_init_e;

/* NOTE: Module definitions. */

exception_init_e    exception_init          (void);
bool                exception_add_handler   (const exception_table_entry *new_entry, exception_handler_f *parent_handler);
void *              exception_handler       (register_stack *stack);

#endif
