#include <stdio.h>

void foo(int *ptr) {
    *ptr = 42;
}

int main() {
    int *init = NULL;
    foo(init);
    return 0;
}
