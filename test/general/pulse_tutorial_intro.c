#include <stdint.h>
#include <stdlib.h>


REQUIRES("x |-> 'i")
REQUIRES("pure (fits (+) (as_int 'i) 1)")
ENSURES("exists* j. (x |-> j) ** pure (as_int 'i + 1 == as_int j)")
void incr (int *x)
{
    *x = *x + 1;
}

ERASED_ARG(#i:_)
REQUIRES(x |-> i)
REQUIRES(pure (fits (+) (as_int i) 1))
ENSURES("exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)")
void incr_explicit_i (int *x)
{
    *x = *x + 1;
}

ERASED_ARG(#i:_)
ERASED_ARG(#j:_)
REQUIRES(x |-> i)
REQUIRES(y |-> j)
REQUIRES(pure (fits (+) (as_int i) 1))
ENSURES("exists* j. (x |-> j) ** pure (as_int i + 1 == as_int j)")
ENSURES(y |-> j)
void incr_frame(int *x, int *y)
{
    incr(x);
}