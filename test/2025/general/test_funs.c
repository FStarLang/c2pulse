#include <stdlib.h>
#include <stdio.h>
#include "../include/PulseMacros.h"
#include "../include/LibcFunctions.h"


RETURNS(Int32.t)
int foo(int a, int b);

RETURNS(Int32.t)
int bar(int c);

int bar(int c){
    int cc = foo(c, c);
    return cc;
}


int foo(int a, int b){
    int abs_a = abs(a );
    int res = bar(abs_a);
    return res;
}
