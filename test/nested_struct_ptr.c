#include "c2pulse.h"
#include <stdint.h>

typedef struct {
    uint32_t first;
    uint32_t second;
} u32_pair;

typedef struct {
    uint64_t first;
    u32_pair *second;
} u64_nested_pair;
