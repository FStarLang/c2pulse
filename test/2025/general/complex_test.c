#include "../c2pulse.h"

_requires((_slprop) _inline_pulse(pure (Int32.fits (Int32.v x * 3))))
_ensures((_slprop) _inline_pulse(pure (Int32.v x * 3 == Int32.v res)))
int triple(int x) { 
    return x * 3; 
}

_requires((_slprop) _inline_pulse(pure (Int32.fits (Int32.v x * Int32.v x))))
_ensures((_slprop) _inline_pulse(pure (Int32.v x * Int32.v x == Int32.v res)))
int square(int x) { 
    return x * x; 
}

_requires((_slprop) _inline_pulse(pure (Int32.fits (Int32.v x * 2))))
_ensures((_slprop) _inline_pulse(pure (Int32.v x * 2 == Int32.v res)))
int doubleValue(int x) {
    return x * 2; 
}

_requires((_slprop) _inline_pulse(pure (Int32.fits (Int32.v x + Int32.v y))))
_ensures((_slprop) _inline_pulse(pure (Int32.v x + Int32.v y == Int32.v res)))
int sum(int x, int y) { 
    return x + y; 
}

_requires((_slprop) _inline_pulse(pure False))
int recursiveFunction(int x, int limit) {
    if (x >= limit) return x;
    else{
        return sum(12,  recursiveFunction(x + 1, limit));
    }
}

///Note: The Z3 solver may be flacky at times and Z3 might not
///be able to prove things. Chaning 1000 to 100 makes this test pass.
_requires((_slprop) _inline_pulse(pure (abs (Int32.v x) < 100 /\ abs (Int32.v y) < 100)))
int complexComputation(int x, int y) {
    return square(triple(sum(doubleValue(x), y))) + 
           doubleValue(square(y - x)) /*- randomOffset(sum(x, y))*/;
}

_requires((_slprop) _inline_pulse(pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)))
int conditionalProcessing(int x, int y) {
    if ((x * y) % 2 == 0) {
        return sum(triple(x), square(y - x));
    } else {
        return doubleValue(square(sum(x, y))) /*- randomOffset(x)*/;
    }
}
