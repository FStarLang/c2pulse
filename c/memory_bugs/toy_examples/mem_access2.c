#include <stdio.h>

void uninitialized() {
    int x;
    printf("%d\n", x);  // UB: x is uninitialized.
}
