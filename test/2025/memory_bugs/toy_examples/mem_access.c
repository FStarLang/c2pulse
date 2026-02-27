#include <stdlib.h>
#include <stdint.h>
#include "../../c2pulse.h"

void buffer_overflow() {
    int *arr = (int*) malloc(5 * sizeof(int));
    arr[10] = 42;  // UB: Out-of-bounds access.
    free(arr);
}
