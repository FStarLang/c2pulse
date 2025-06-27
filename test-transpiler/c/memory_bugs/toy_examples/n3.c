#include <stdio.h>
#include <stdlib.h>

void unsafe_function(int *ptr) {
    *ptr = 100;
    printf("Value: %d\n", *ptr);
}

int main() {
    int *ptr = NULL;
    unsafe_function(ptr);
    return 0;
}
