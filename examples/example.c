#include "c2pulse.h"

void swap(int *x, int *y)
    _ensures(*y == _old(*x) && *x == _old(*y))
{
    int tmp = *y;
    *y = *x;
    *x = tmp;
}

int max(int a, int b)
    _ensures(a <= return && b <= return)
{
    if (a <= b) {
        return b;
    } else {
        return a;
    }
}