/*  serial.c

DESCRIPTION

    Serial code for use while inside an exception.

    Almost all of this has been ripped verbatim from libdream/KOS.

CHANGELOG

    Sat Feb  9 16:32:19 PST 2002    Scott Robinson <scott_vo@dsn.itgo.com>
        First added this changelog.

*/

#include "vars.h"

#include "serial.h"

void serial_set_baudrate(uint16 baud_rate)
{
    /* STAGE: 8N1, use P0 clock */
    *SCIF_R_SMR = 0;

    /* STAGE: Set baudrate, N = P0/(32*B)-1 */
    *SCIF_R_BRR = (50000000 / (32 * baud_rate)) - 1;
}
