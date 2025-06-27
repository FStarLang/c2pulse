#include <stdio.h>
#include <stdlib.h>

int main() {
    int *ptr = NULL;

    free(ptr);
    free(ptr);

    ptr = malloc(sizeof(int));
    free(ptr);
    free(ptr);
    
    return 0;
}
