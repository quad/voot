#include "serial.h"

/* Initialize the SCIF port; baud_rate must be at least 9600 and
   no more than 57600. 115200 does NOT work for most PCs. */
void ubc_serial_init(int baud_rate) {
	volatile unsigned short *scif16 = (unsigned short*)0xffe80000;
	volatile unsigned char *scif8 = (unsigned char*)0xffe80000;

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
void ubc_serial_write(int c) {
	volatile unsigned short *ack = (unsigned short*)0xffe80010;
	volatile unsigned char *fifo = (unsigned char*)0xffe8000c;
	
	/* Wait until the transmit buffer has space */
	while (!(*ack & 0x20))
		;
	
	/* Send the char */
	*fifo = c;
	
	/* Clear status */
	*ack &= 0x9f;
}

/* Flush all FIFO'd bytes out of the serial port buffer */
void ubc_serial_flush() {
	volatile unsigned short *ack = (unsigned short*)0xffe80010;

	*ack &= 0xbf;
	while (!(*ack & 0x40))
		;
	*ack &= 0xbf;
}

/* Send an entire buffer */
void ubc_serial_write_buffer(unsigned char *data, int len) {
	while (len-- > 0)
		ubc_serial_write(*data++);
	ubc_serial_flush();
}

/* Send a string (null-terminated) */
void ubc_serial_write_str(char *str) {
	ubc_serial_write_buffer((unsigned char*)str, strlen(str));
}
