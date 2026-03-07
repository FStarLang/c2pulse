#include "c2pulse.h"
#include <stdlib.h>

typedef struct {
    int x;
    int y;
} point;

_pure int zero;

_pure point zero_point;

void test_calloc_ref(void) {
    int *p = (int *) calloc(1, sizeof(int));
    _assert(*p == 0);
    free(p);
}
