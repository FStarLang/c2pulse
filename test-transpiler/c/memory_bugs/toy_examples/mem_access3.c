#include <stdlib.h>

void use_after_free() {
    int *p = malloc(sizeof(int));
    free(p);
    *p = 100;  // UB: Accessing freed memory.
}
