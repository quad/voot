#ifndef __BIUDP_H__
#define __BIUDP_H__

#include "vars.h"
#include "net.h"

#define BIUDP_SEGMENT_SIZE  1024

typedef struct
{
    uint8   dest_mac[ETHER_MAC_SIZE];
    uint32  source_ip;
    uint32  dest_ip;
    uint32  port;
    bool    initialized;
} biudp_control_t;

void biudp_init(const biudp_control_t *in_control);
void biudp_write_buffer(const uint8 *in_data, uint32 in_data_length);
void biudp_write(const uint8 in);
void biudp_write_str(const uint8 *in_string);
void biudp_write_hex(uint32 val);

/* SERIAL DEFS
    void    ubc_serial_init         (uint16 baud_rate);
    void    ubc_serial_write        (int32 c);
    void    ubc_serial_write_buffer (uint8 * data, uint32 len);
    void    ubc_serial_write_str    (uint8 * str);
    int32   ubc_serial_read         (void);
    void    ubc_serial_write_hex    (uint32 val);
*/

#endif
