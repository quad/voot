/*  serial.c

    $Id: serial.c,v 1.4 2002/06/30 09:15:06 quad Exp $

DESCRIPTION

    Serial code for use while inside an exception.

TODO

    Translate the raw values in the prototype functions to symbolic
    assignments. (placed in the header file)

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

void serial_write_char (char c)
{
    /* STAGE: Wait until the transmit buffer has space. */

    while (!(*SCIF_R_FS & 0x20));

    /* STAGE: Send the character. */

    *SCIF_R_TD = c;

    /* STAGE: Clear the status. */

    *SCIF_R_FS &= 0x9f;
}

void serial_flush (void)
{
    *SCIF_R_FS &= 0xbf;

    while (!(*SCIF_R_FS & 0x40));

    *SCIF_R_FS &= 0xbf;
}

void serial_write_buffer (uint8 *data, uint32 len)
{
    while (len-- > 0)
        serial_write_char (*data++);

    serial_flush ();
}

int32 serial_read_char (void)
{
    int32 c;

    c = -1;

    /* STAGE: Buffer overrun? */

    if (*SCIF_R_LS & SCIF_LS_ORER)
        *SCIF_R_LS &= ~(SCIF_LS_ORER);

    /* STAGE: Error and break handling. */    

    else if (*SCIF_R_FS & (SCIF_FS_ER | SCIF_FS_BRK))
        *SCIF_R_FS &= ~(SCIF_FS_ER | SCIF_FS_BRK);

    /* STAGE: Check if we have a character waiting for us. */

    else if (*SCIF_R_FS & (SCIF_FS_RDF | SCIF_FS_DR))
    {
        /* STAGE: Get the character. */

        c = *SCIF_R_RD;

        /* STAGE: Flush the character out. */

        *SCIF_R_FS &= ~(SCIF_FS_RDF | SCIF_FS_DR);
    }

    return c;
}

