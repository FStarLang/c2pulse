#include "../include/PulseMacros.h"

REQUIRES(pure (Int32.fits (Int32.v x * 3)))
RETURNS(res : Int32.t)
ENSURES(pure (Int32.v x * 3 == Int32.v res))
int triple(int x) { 
    return x * 3; 
}

REQUIRES(pure (Int32.fits (Int32.v x * Int32.v x)))
RETURNS(res : Int32.t)
ENSURES(pure (Int32.v x * Int32.v x == Int32.v res))
int square(int x) { 
    return x * x; 
}

REQUIRES(pure (Int32.fits (Int32.v x * 2)))
RETURNS(res : Int32.t)
ENSURES(pure (Int32.v x * 2 == Int32.v res))
int doubleValue(int x) { 
    return x * 2; 
}


REQUIRES(pure (Int32.fits (Int32.v x + Int32.v y)))
RETURNS(res : Int32.t)
ENSURES(pure (Int32.v x + Int32.v y == Int32.v res))
int sum(int x, int y) { 
    return x + y; 
}

REQUIRES(pure False)
RETURNS(res : Int32.t)
int recursiveFunction(int x, int limit) {
    if (x >= limit) return x;
    else{
        return sum(12,  recursiveFunction(x + 1, limit));
    }
}

REQUIRES(pure (abs (Int32.v x) < 1000 /\\ abs (Int32.v y) < 1000))
RETURNS(res : Int32.t)
int complexComputation(int x, int y) {
    return square(triple(sum(doubleValue(x), y))) + 
           doubleValue(square(y - x)) /*- randomOffset(sum(x, y))*/;
}

REQUIRES(pure (abs (Int32.v x) < 1000 /\\ abs (Int32.v y) < 1000))
RETURNS(res : Int32.t)
int conditionalProcessing(int x, int y) {
    if ((x * y) % 2 == 0) {
        return sum(triple(x), square(y - x));
    } else {
        return doubleValue(square(sum(x, y))) /*- randomOffset(x)*/;
    }
}
