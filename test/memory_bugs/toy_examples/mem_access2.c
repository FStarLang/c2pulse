#include <stdlib.h>
#include "../../include/PulseMacros.h"

REQUIRES(emp)
ENSURES(emp)
void uninitialized() {
    int *x = (int *) malloc(sizeof(int));
    *x = 1;
    int *x1 = x;
    free(x);
    return; 
}

int main(){
uninitialized();
    return 0;
}