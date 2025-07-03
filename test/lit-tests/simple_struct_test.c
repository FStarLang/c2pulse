// RUN: %c2pulse %s
// RUN: cat %p/Simple_struct_test.fst
// RUN: diff %p/Simple_struct_test.fst %p/../snapshots/Simple_struct_test.fst
// RUN: %run_fstar.sh %p/Simple_struct_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE
#include <stdint.h>
#include <stdlib.h>
#include "../../include/PulseMacros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;



// PULSE: All verification conditions discharged successfully
