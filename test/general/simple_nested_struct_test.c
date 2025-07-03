#include <stdint.h>
#include <stdlib.h>
#include "../include/PulseMacros.h"

typedef struct _u32_pair_struct {
  uint32_t first;
  uint32_t second;
} u32_pair_struct;

typedef struct _u64_pair_struct {
  uint64_t first;
  u32_pair_struct *second;
} u64_pair_struct;
