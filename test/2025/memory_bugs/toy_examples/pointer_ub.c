#include <stddef.h>
#include "../../c2pulse.h"

//_requires((_slprop) _inline_pulse(p |-> v))
_ensures((_slprop) _inline_pulse(exists* (w: Pulse.Lib.C.Int32.int32).
        (p |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == 1)))
void null_deref() {
    int *p = NULL;
    *p = 1;  // UB: Dereferencing NULL.
}