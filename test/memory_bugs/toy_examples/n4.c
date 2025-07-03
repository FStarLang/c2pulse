#include <stdio.h>
#include <stdlib.h>

int main() {
    int *ptr = NULL;

    if (ptr != NULL) {
        *ptr = 20;
        printf("Value: %d\n", *ptr);
    } else {
        printf("Pointer is NULL\n");
    }

    return 0;
}
