/*  serial.h

    $Id: serial.h,v 1.1 2002/06/11 20:16:39 quad Exp $

*/

#ifndef __COMMON_SERIAL_H__
#define __COMMON_SERIAL_H__

#include "vars.h"

#define _SCIF_BASE      (0xFFE80000)
#define SCIF_R_SMR      (REGISTER(uint16)   (_SCIF_BASE + 0x00))
#define SCIF_R_BRR      (REGISTER(uint8)    (_SCIF_BASE + 0x04))
#define SCIF_R_SC       (REGISTER(uint16)   (_SCIF_BASE + 0x08))
#define SCIF_R_FTG      (REGISTER(uint8)    (_SCIF_BASE + 0x0C))
#define SCIF_R_FS       (REGISTER(uint16)   (_SCIF_BASE + 0x10))
#define SCIF_R_FRD      (REGISTER(uint8)    (_SCIF_BASE + 0x14))
#define SCIF_R_FC       (REGISTER(uint16)   (_SCIF_BASE + 0x18))

#define SCIF_SC_RIE     (1<<6)

#define SCIF_FS_DR      (1)
#define SCIF_FS_RDF     (1<<1)
#define SCIF_FS_TDFE    (1<<5)
#define SCIF_FS_TEND    (1<<6)

#define SCIF_FC_LOOP    (1)

#define SCIF_TIMEOUT    10000

void    serial_set_baudrate (uint16 baud_rate);

#endif
