/*  serial.c

    $Id: serial.c,v 1.2 2002/06/20 10:20:05 quad Exp $

DESCRIPTION

    Serial code for use while inside an exception.

*/

#include "vars.h"

#include "serial.h"

void serial_set_baudrate (uint16 baud_rate)
{
    /* STAGE: 8N1, use P0 clock */
    *SCIF_R_SM = 0;

    /* STAGE: Set baudrate, N = P0/(32*B)-1 */
    *SCIF_R_BR = (50000000 / (32 * baud_rate)) - 1;
}
