/*  serial.h

    $Id: serial.h,v 1.4 2002/06/30 09:15:06 quad Exp $

*/

#ifndef __COMMON_SERIAL_H__
#define __COMMON_SERIAL_H__

#include "vars.h"

#define _SCIF_BASE      (0xFFE80000)

#define SCIF_R_SM       (REGISTER(uint16)   (_SCIF_BASE + 0x00))
#define SCIF_R_BR       (REGISTER(uint8)    (_SCIF_BASE + 0x04))
#define SCIF_R_SC       (REGISTER(uint16)   (_SCIF_BASE + 0x08))
#define SCIF_R_TD       (REGISTER(uint8)    (_SCIF_BASE + 0x0C))
#define SCIF_R_FS       (REGISTER(uint16)   (_SCIF_BASE + 0x10))
#define SCIF_R_RD       (REGISTER(uint8)    (_SCIF_BASE + 0x14))
#define SCIF_R_FC       (REGISTER(uint16)   (_SCIF_BASE + 0x18))
#define SCIF_R_DR       (REGISTER(uint16)   (_SCIF_BASE + 0x1C))
#define SCIF_R_LS       (REGISTER(uint16)   (_SCIF_BASE + 0x24))

#define SCIF_SC_RIE     (1<<6)

#define SCIF_FS_DR      (1)
#define SCIF_FS_RDF     (1<<1)
#define SCIF_FS_BRK     (1<<4)
#define SCIF_FS_TDFE    (1<<5)
#define SCIF_FS_TEND    (1<<6)
#define SCIF_FS_ER      (1<<7)

#define SCIF_FC_LOOP    (1)

#define SCIF_LS_ORER    (1)

#define SCIF_TIMEOUT    10000

/* NOTE: Module definitions. */

void    serial_set_baudrate (uint16 baud_rate);
void    serial_write_char   (char c);
void    serial_flush        (void);
void    serial_write_buffer (uint8 *data, uint32 len);
int32   serial_read_char    (void);

#endif
