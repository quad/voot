#ifndef __PRINTF_H__
#define __PRINTF_H__

#include <stdarg.h>

int vsnprintf(char *buf, uint32 size, const char *fmt, va_list args);
int snprintf(char *buf, uint32 size, const char *fmt, ...);

#endif
