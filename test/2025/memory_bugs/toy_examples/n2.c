#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));

    int *ptr;
    if (rand() % 2) {
        ptr = malloc(sizeof(int));
        *ptr = 42;
    } else {
        ptr = NULL;
    }

    *ptr = 99;
    free(ptr);
    return 0;
}
