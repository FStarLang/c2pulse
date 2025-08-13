#include <stddef.h>
#include "../../include/PulseMacros.h"

// EXPECT_FAILURE()
REQUIRES(p |-> v)
ENSURES(exists* (w: Pulse.Lib.C.Int32.int32).
        (p |-> w) ** pure Pulse.Lib.C.Int32.(as_int w == 1))
void null_deref() {
    int *p = NULL;
    *p = 1;  // UB: Dereferencing NULL.
}