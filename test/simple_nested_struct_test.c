// RUN: %c2pulse %s 
// RUN: cat %p/Simple_nested_struct_test/Module_u32_pair_struct.fsti | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: cat %p/Simple_nested_struct_test/Module_u64_pair_struct.fsti | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Simple_nested_struct_test/Module_u32_pair_struct.fsti %p/Simple_nested_struct_test/Module_u64_pair_struct.fsti 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include <stdint.h>
#include <stdlib.h>
#include "../../test-transpiler/c/pulse_macros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

typedef struct _u64_pair_struct {
  uint64_t first;
  u32_pair_struct *second;
} u64_pair_struct;

// COM: ===========================================================================

// PULSE: All verification conditions discharged successfully