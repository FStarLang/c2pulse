#include <stdio.h>
#include <stdlib.h>

struct Data {
    int *ptr;
};

int main() {
    struct Data d;
    d.ptr = NULL;

    *d.ptr = 10;

    if (d.ptr != NULL) {
        *d.ptr = 20;
    }

    return 0;
}
