/*  assert.h

    $Id: assert.h,v 1.1 2002/06/11 20:33:06 quad Exp $

*/

#ifndef __COMMON_ASSERT_H__
#define __COMMON_ASSERT_H__

#define assert(e)           ((e) ? (void) 0 : __assert (__FILE__, __LINE__, #e, __FUNCTION__, 0))
#define assert_x(e,x)       ((e) ? (void) 0 : __assert (__FILE__, __LINE__, #e, __FUNCTION__, (uint32) x))

void    __assert    (const char *module, int32 line, const char *expr, const char *func, uint32 extra);

#endif
