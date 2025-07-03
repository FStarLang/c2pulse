#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));

    int *p1 = malloc(sizeof(int));
    int *p2 = NULL;
    int *p3;

    *p1 = 42;

    if (rand() % 2) {
        p2 = malloc(sizeof(int));
        *p2 = 100;
    }

    if (p2 != NULL) {
        *p2 = 200;
    } else {
        *p2 = 300;
    }

    free(p1);
    free(p2);

    return 0;
}
