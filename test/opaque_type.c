#include "pal.h"
#include <stdint.h>

// Define an opaque F* type alias
_type(my_list, list Int32.t)

// Use it in _include_pulse via $type
_include_pulse(
  let empty_list : $type(my_list) = ([] #Int32.t)
)

// Use it as a _let return type with _inline_pulse in the body
_let(my_list make_singleton(int x),
  (my_list) _inline_pulse(
    [$(x)] <: $type(my_list)
  )
)
