#ifndef __ASSERT_H__
#define __ASSERT_H__

#define assert(e)           ((e) ? (void)0 : __assert(__FILE__, __LINE__, #e, __FUNCTION__))

void __assert(const char *module, int32 line, const char *expr, const char *func);

#endif
