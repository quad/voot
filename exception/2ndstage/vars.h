/*  vars.h

    Ok, I give in. I'm not going to type out every variable type anymore.
*/

#ifndef __VARS_H__
#define __VARS_H__

#define NULL            0x0

#define REGISTER(x)     (volatile x *)

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef enum {FALSE, TRUE} bool;

#endif
