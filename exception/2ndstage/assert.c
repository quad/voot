/*  assert.c

DESCRIPTION

    Assertion logic. Since in a state of panic, we can't be sure of the
    networking logic, the module uses the biosfont sub-system and panics
    on-screen.

*/

#include "vars.h"
#include "biosfont.h"

#include "assert.h"
