/*  vars.h

    $Id: vars.h,v 1.2 2002/10/18 19:52:19 quad Exp $

*/

#ifndef __COMMON_VARS_H__
#define __COMMON_VARS_H__

#ifndef NULL
    #define NULL        0x0
#endif

#define REGISTER(x)     (volatile x *)

typedef signed char     int8;
typedef signed short    int16;
typedef signed int      int32;

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef enum
{
    FALSE,
    TRUE
} bool;

#endif
