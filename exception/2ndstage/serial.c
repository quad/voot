/*  serial.c

    Serial code for use while inside an exception.

    Almost all of this has been ripped verbatim from libdream/KOS.
*/

#include "vars.h"
#include "serial.h"
#include "util.h"

/* Initialize the SCIF port; baud_rate must be at least 9600 and
   no more than 57600. 115200 does NOT work for most PCs. */
void ubc_serial_init(uint16 baud_rate)
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
void ubc_serial_write(int32 c)
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
void ubc_serial_flush(void)
{
    volatile uint16 *ack = (uint16 *) 0xffe80010;

    *ack &= 0xbf;

    while (!(*ack & 0x40));

    *ack &= 0xbf;
}

/* Send an entire buffer */
void ubc_serial_write_buffer(uint8 * data, uint32 len)
{
    while (len-- > 0)
        ubc_serial_write(*data++);

    ubc_serial_flush();
}

/* Send a string (null-terminated) */
void ubc_serial_write_str(uint8 * str)
{
    ubc_serial_write_buffer((uint8 *) str, strlen(str));
}

/* Read one char from the serial port (-1 if nothing to read) */
int32 ubc_serial_read(void)
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

void ubc_serial_write_hex(uint32 val)
{
    uint8 uint_buffer[10];

    uint_to_string(val, uint_buffer);

    ubc_serial_write_str(uint_buffer);
}
