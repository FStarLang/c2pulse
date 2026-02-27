#include <stdlib.h>
#include "../../c2pulse.h"

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