#include <stdio.h>
#include <stdlib.h>

int main() {
    int *arr[3];
    arr[0] = malloc(sizeof(int));
    arr[1] = NULL;
    arr[2] = malloc(sizeof(int));

    *arr[0] = 10;
    *arr[1] = 20;
    *arr[2] = 30;

    free(arr[0]);
    free(arr[1]);
    free(arr[2]);

    return 0;
}
