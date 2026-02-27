#include <stdlib.h>
#include <stdio.h>
#include "../c2pulse.h"

int foo(int a, int b);

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
