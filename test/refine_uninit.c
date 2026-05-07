// Test: _refine_uninit — refinement that only applies to uninit predicates.
#include "pal.h"
#include <stdint.h>

_refine_uninit((_slprop) _inline_pulse(pure (is_null $(this))))
typedef int *nullable_ptr;

void take_nullable(_plain nullable_ptr p)
    _requires((_slprop) _inline_pulse(pure (is_null $(p))))
    _ensures((_slprop) _inline_pulse(pure (is_null $(p))))
{
}
