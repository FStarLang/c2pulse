#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../c2pulse.h"

typedef struct _foo{
int a; 
int b;
};

union ab {
    int a; 
    bool b;
};

uint32_t foo(){
    return 10;
}

long long foo1(){
    return 10;
}

long foo2(){
    return 10;
}

int* foo3(){
    int *x;
    return x;
}

uint32_t* foo4(){
    uint32_t *x;
    return x;
}

void foo5(){
    return;
}

//Vidush: Should we just fail for all functions 
//that return a void pointer?
void *foo6(){
    return NULL;
}

bool foo7(){
    return false;
}

int32_t bar(){
    return 10;
}

struct _foo *baz(){
    return NULL;
}

_requires((_slprop) _inline_pulse(pure (Case_ab_a? s)))
union ab *baz1(union ab *x){
    return x;
}
