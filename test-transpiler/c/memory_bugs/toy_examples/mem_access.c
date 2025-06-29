#include <stdio.h>

void buffer_overflow() {
    int arr[5];
    arr[10] = 42;  // UB: Out-of-bounds access.
}
