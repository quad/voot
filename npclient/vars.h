/*  vars.h

DESCRIPTION

    Various data type declarations made a bit more specific for portability's sake.

CHANGELOG

    Tue Jan 22 17:50:20 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added this changelog entry. The file has been around for quite some time.

*/

#ifndef __VARS_H__
#define __VARS_H__

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef enum {FALSE, TRUE} bool;

#endif __VARS_H__
