#include "pal.h"
#include <stdint.h>

// Define an opaque F* type alias
_type(my_list, list Int32.t)

// Use it in _include_pulse via $type
_include_pulse(
  let empty_list : $type(my_list) = ([] #Int32.t)
)
