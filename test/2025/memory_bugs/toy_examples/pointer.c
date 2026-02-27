#include <stddef.h>
#include "../../include/PulseMacros.h"

ERASED_ARG(#v: Pulse.Lib.C.Int32.int32)   
REQUIRES(p |-> v)
ENSURES(exists* (w: Pulse.Lib.C.Int32.int32).
        (p |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == 1))
void deref(int *p) {
    *p = 1;
    p = NULL;
}
