// Test for _refine_always: refinement that applies to both init and uninit predicates.
// The array length refinement must be preserved in _out mode's uninit precondition.

#include "c2pulse.h"

_refine_always(this._length == 4) typedef _array unsigned *four_element_array;

// Regular parameter: refinement in both pre and post (init variant)
unsigned read_first(four_element_array arr)
{
    return arr[0];
}

// _out parameter: refinement appears in uninit precondition too
void populate(_out four_element_array arr) {
    // Use ghost_stmt to admit — writing to uninit arrays requires mask_write
    _ghost_stmt(admit ());
}
