#ifndef __PRINTF_H__
#define __PRINTF_H__

#include <stdarg.h>

#define N_ZEROPAD   1       /* pad with zero */
#define N_SIGN      2       /* unsigned/signed long */
#define N_PLUS      4       /* show plus */
#define N_SPACE     8       /* space if plus */
#define N_LEFT      16      /* left justified */
#define N_SPECIAL   32      /* 0x */
#define N_LARGE     64      /* use 'ABCDEF' instead of 'abcdef' */

char* number(char *str, long num, int32 base, int32 size, int32 precision, int32 type);
int vsnprintf(char *buf, uint32 size, const char *fmt, va_list args);
int snprintf(char *buf, uint32 size, const char *fmt, ...);

#endif
