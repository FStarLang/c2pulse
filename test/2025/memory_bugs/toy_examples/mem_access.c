#include <stdlib.h>
#include <stdint.h>
#include "../../include/PulseMacros.h"


void buffer_overflow() {
    ISARRAY(5) int *arr = (int*) malloc(5 * sizeof(int));
    arr[10] = 42;  // UB: Out-of-bounds access.
    free(arr);
}
