#include <stdlib.h>

void leak() {
    int *p = malloc(10 * sizeof(int));
    /* forgot to free(p); */
}
