/*  serial.c

DESCRIPTION
    Non-crashing serial code for use while inside an exception.

    Almost all of this has been ripped verbatim from libdream/KOS.

COPYING
    See "COPYING" in the root directory of the distribution.

CHANGELOG
    Mon Aug  6 15:29:30 PDT 2001    Dan Potter <bard-dcdev@allusion.net>
        Dan Potter didn't really write in this particular source file, but
        this adds him as a copyright holder for this file.

    Mon Aug  6 15:46:17 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        Imported, modified, and just generally added a timestamp when I
        created the libdc distribution.

    Wed Aug  8 00:17:14 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        Removed the "ubc_" prefix to all the functions. This solves two
        problems: 1) brings the naming of functions back into my current
        paradigm. 2) it breaks any old debugging code! Mu ha ha!

*/

#include "vars.h"
#include "util.h"
#include "serial.h"

/* Initialize the SCIF port; baud_rate must be at least 9600 and
   no more than 57600. 115200 does NOT work for most PCs. */
void serial_init(uint16 baud_rate)
{
    volatile uint16 *scif16 = (uint16 *) 0xffe80000;
    volatile uint8 *scif8 = (uint8 *) 0xffe80000;

    /* Disable interrupts, transmit/receive, and use internal clock */
    scif16[8 / 2] = 0;

    /* 8N1, use P0 clock */
    scif16[0] = 0;

    /* Set baudrate, N = P0/(32*B)-1 */
    scif8[4] = (50000000 / (32 * baud_rate)) - 1;

    /* Reset FIFOs, enable hardware flow control */
    scif16[24 / 2] = 12;
    scif16[24 / 2] = 8;

    /* Disable manual pin control */
    scif16[32 / 2] = 0;

    /* Clear status */
    scif16[16 / 2] = 0x60;
    scif16[36 / 2] = 0;

    /* Enable transmit/receive */
    scif16[8 / 2] = 0x30;
}

/* Write one char to the serial port (call serial_flush()!)*/
void serial_write(int32 c)
{
    volatile uint16 *ack = (uint16 *) 0xffe80010;
    volatile uint8 *fifo = (uint8 *) 0xffe8000c;

    /* Wait until the transmit buffer has space */
    while (!(*ack & 0x20));

    /* Send the char */
    *fifo = c;

    /* Clear status */
    *ack &= 0x9f;
}

/* Flush all FIFO'd bytes out of the serial port buffer */
void serial_flush(void)
{
    volatile uint16 *ack = (uint16 *) 0xffe80010;

    *ack &= 0xbf;

    while (!(*ack & 0x40));

    *ack &= 0xbf;
}

/* Send an entire buffer */
void serial_write_buffer(uint8 * data, uint32 len)
{
    while (len-- > 0)
        serial_write(*data++);

    serial_flush();
}

/* Send a string (null-terminated) */
void serial_write_str(uint8 * str)
{
    serial_write_buffer((uint8 *) str, strlen(str));
}

/* Read one char from the serial port (-1 if nothing to read) */
int32 serial_read(void)
{
    volatile uint16 *status = (uint16 *) 0xffe8001c;
    volatile uint16 *ack = (uint16 *) 0xffe80010;
    volatile uint8 *fifo = (uint8 *) 0xffe80014;
    int32 c;

    /* Check input FIFO */
    if (!(*status & 0x1f))
        return -1;

    /* Get the input char */
    c = *fifo;

    /* Ack */
    *ack &= 0x6d;

    return c;
}

void serial_write_hex(uint32 val)
{
    uint8 uint_buffer[10];

    uint_to_string(val, uint_buffer);

    serial_write_str(uint_buffer);
}
