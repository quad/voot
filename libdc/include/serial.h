#ifndef __LIBDC_SERIAL_H__
#define __LIBDC_SERIAL_H__

#include "vars.h"

void    serial_init         (uint16 baud_rate);
void    serial_write        (int32 c);
void    serial_flush        (void);
void    serial_write_buffer (uint8 * data, uint32 len);
void    serial_write_str    (uint8 * str);
int32   serial_read         (void);
void    serial_write_hex    (uint32 val);

#endif
