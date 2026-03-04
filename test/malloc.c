#include "c2pulse.h"
#include <stdlib.h>

void test_malloc_free(void) {
    int *p = (int *) malloc(sizeof(int));
    *p = 42;
    free(p);
}
