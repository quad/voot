#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "vars.h"

void    ubc_serial_init         (uint16 baud_rate);
void    ubc_serial_write        (int32 c);
void    ubc_serial_flush        (void);
void    ubc_serial_write_buffer (uint8 * data, uint32 len);
void    ubc_serial_write_str    (uint8 * str);
int32   ubc_serial_read         (void);
void    ubc_serial_write_hex    (uint32 val);

#endif
