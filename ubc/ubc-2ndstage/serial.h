#ifndef __SERIAL_H__
#define __SERIAL_H__

void ubc_serial_init(int baud_rate);
void ubc_serial_write(int c);
void ubc_serial_flush();
void ubc_serial_write_buffer(unsigned char *data, int len);
void ubc_serial_write_str(char *str);

#endif