/*  assert.h

    $Id: assert.h,v 1.2 2002/06/12 10:29:01 quad Exp $

*/

#ifndef __COMMON_ASSERT_H__
#define __COMMON_ASSERT_H__

#define assert(e)           ((e) ? (void) 0 : __assert (__FILE__, __LINE__, #e, __FUNCTION__, 0))
#define assert_x(e,x)       ((e) ? (void) 0 : __assert (__FILE__, __LINE__, #e, __FUNCTION__, (uint32) x))

void    __assert    (const char *module, int32 line, const char *expr, const char *func, uint32 extra)  __attribute__ ((noreturn));

#endif
