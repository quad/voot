/* This file is part of the libdream Dreamcast function library.
   Please see libdream.c for further details.
   
   serial.c
   (c)2000 Dan Potter
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "dream.h"

/*

This module handles serial I/O using the SH4's SCIF port. FIFO mode
is used by default; you can turn this off to avoid forcing a wait
when there is no serial device attached.

Ported from KallistiOS (Dreamcast OS) for libdream by Dan Potter

*/

/* Pre-disable serial output */
int serial_disabled = 0;
void serial_disable() {
	serial_disabled = 1;
}

/* Initialize the SCIF port; baud_rate must be at least 9600 and
   no more than 57600. 115200 does NOT work for most PCs. */
void serial_init(int baud_rate) {
	volatile uint16 *scif16 = (uint16*)0xffe80000;
	volatile uint8 *scif8 = (uint8*)0xffe80000;

	if (serial_disabled) return;
	
	/* Disable interrupts, transmit/receive, and use internal clock */
	scif16[8/2] = 0;
	
	/* 8N1, use P0 clock */
	scif16[0] = 0;
	
	/* Set baudrate, N = P0/(32*B)-1 */
	scif8[4] = (50000000 / (32 * baud_rate)) - 1;

	/* Reset FIFOs, enable hardware flow control */	
	scif16[24/2] = 12;
	scif16[24/2] = 8;
	
	/* Disable manual pin control */
	scif16[32/2] = 0;
	
	/* Clear status */
	scif16[16/2] = 0x60;
	scif16[36/2] = 0;
	
	/* Enable transmit/receive */
	scif16[8/2] = 0x30;
}

/* Write one char to the serial port (call serial_flush()!)*/
void serial_write(int c) {
	volatile uint16 *ack = (uint16*)0xffe80010;
	volatile uint8 *fifo = (uint8*)0xffe8000c;
	
	if (serial_disabled) return;
	
	/* Wait until the transmit buffer has space */
	while (!(*ack & 0x20))
		;
	
	/* Send the char */
	*fifo = c;
	
	/* Clear status */
	*ack &= 0x9f;
}

/* Flush all FIFO'd bytes out of the serial port buffer */
void serial_flush() {
	volatile uint16 *ack = (uint16*)0xffe80010;

	if (serial_disabled) return;
	
	*ack &= 0xbf;
	while (!(*ack & 0x40))
		;
	*ack &= 0xbf;
}

/* Send an entire buffer */
void serial_write_buffer(uint8 *data, int len) {
	while (len-- > 0)
		serial_write(*data++);
	serial_flush();
}

/* Send a string (null-terminated) */
void serial_write_str(char *str) {
	serial_write_buffer((uint8*)str, strlen(str));
}

/* Read one char from the serial port (-1 if nothing to read) */
int serial_read() {
	volatile uint16 *status = (uint16*)0xffe8001c;
	volatile uint16 *ack = (uint16*)0xffe80010;
	volatile uint8 *fifo = (uint8*)0xffe80014;
	int c;

	if (serial_disabled) return -1;
	
	/* Check input FIFO */
	if (!(*status & 0x1f)) return -1;
	
	/* Get the input char */
	c = *fifo;
	
	/* Ack */
	*ack &= 0x6d;

	return c;
}

/* Printf functionality */
char printf_buf[2048];
int serial_printf(char *fmt, ...) {
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(printf_buf, fmt, args);
	va_end(args);

	serial_write_str(printf_buf);

	return i;
}

