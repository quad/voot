/*  printf.h

    $Id: printf.h,v 1.1 2002/06/11 20:36:55 quad Exp $

*/

#ifndef __COMMON_PRINTF_H__
#define __COMMON_PRINTF_H__

/* NOTE: We broke our no standard header rule. */

#include <stdarg.h>

/* NOTE: Flags which can be passed to number () */

#define N_ZEROPAD   1       /* pad with zero */
#define N_SIGN      2       /* unsigned/signed long */
#define N_PLUS      4       /* show plus */
#define N_SPACE     8       /* space if plus */
#define N_LEFT      16      /* left justified */
#define N_SPECIAL   32      /* 0x */
#define N_LARGE     64      /* use 'ABCDEF' instead of 'abcdef' */

/* NOTE: Module definitions */

char *  number      (char *str, long num, int32 base, int32 size, int32 precision, int32 type);
int     vsnprintf   (char *buf, uint32 size, const char *fmt, va_list args);
int     snprintf    (char *buf, uint32 size, const char *fmt, ...);

#endif
