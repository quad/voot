/*  exception.h 

    $Id: exception.h,v 1.5 2002/06/29 12:57:04 quad Exp $

*/

#ifndef __COMMON_EXCEPTION_H__
#define __COMMON_EXCEPTION_H__

#include "vars.h"
#include "system.h"

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

/* NOTE: Module definitions. */

void    exception_init          (void);
bool    exception_add_handler   (const exception_table_entry *new_entry, exception_handler_f *parent_handler);
void *  exception_handler       (register_stack *stack);

#endif
