// RUN: %c2pulse %s 2>&1 | %{FILECHECK} %s --check-prefix=C2PULSE
// RUN: %run_fstar.sh %p/Complex_test.fst 2>&1 | %{FILECHECK} %s --check-prefix=PULSE

#include "../../test-transpiler/c/pulse_macros.h"


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

// C2PULSE: fn triple
// C2PULSE-NEXT: (x : Int32.t)
// C2PULSE-NEXT: requires pure (Int32.fits (Int32.v x * 3))
// C2PULSE-NEXT: returns res : Int32.t
// C2PULSE-NEXT: ensures pure (Int32.v x * 3 == Int32.v res)
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (Int32.mul x 3l);
// C2PULSE-NEXT: }
// C2PULSE-NEXT: fn square
// C2PULSE-NEXT: (x : Int32.t)
// C2PULSE-NEXT: requires pure (Int32.fits (Int32.v x * Int32.v x))
// C2PULSE-NEXT: returns res : Int32.t
// C2PULSE-NEXT: ensures pure (Int32.v x * Int32.v x == Int32.v res)
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (Int32.mul x x);
// C2PULSE-NEXT: }
// C2PULSE-NEXT: fn doubleValue
// C2PULSE-NEXT: (x : Int32.t)
// C2PULSE-NEXT: requires pure (Int32.fits (Int32.v x * 2))
// C2PULSE-NEXT: returns res : Int32.t
// C2PULSE-NEXT: ensures pure (Int32.v x * 2 == Int32.v res)
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (Int32.mul x 2l);
// C2PULSE-NEXT: }
// C2PULSE-NEXT: fn sum
// C2PULSE-NEXT: (x : Int32.t)
// C2PULSE-NEXT: (y : Int32.t)
// C2PULSE-NEXT: requires pure (Int32.fits (Int32.v x + Int32.v y))
// C2PULSE-NEXT: returns res : Int32.t
// C2PULSE-NEXT: ensures pure (Int32.v x + Int32.v y == Int32.v res)
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (Int32.add x y);
// C2PULSE-NEXT: }
// C2PULSE-NEXT: fn rec recursiveFunction
// C2PULSE-NEXT: (x : Int32.t)
// C2PULSE-NEXT: (limit : Int32.t)
// C2PULSE-NEXT: requires pure False
// C2PULSE-NEXT: returns res : Int32.t
// C2PULSE-NEXT: {
// C2PULSE-NEXT: if((Int32.eq x limit))
// C2PULSE-NEXT: {
// C2PULSE-NEXT: x;
// C2PULSE-NEXT: }
// C2PULSE-NEXT: else
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (sum 12l (recursiveFunction (Int32.add x 1l) limit));
// C2PULSE-NEXT: }
// C2PULSE-NEXT: }
// C2PULSE-NEXT: fn complexComputation
// C2PULSE-NEXT: (x : Int32.t)
// C2PULSE-NEXT: (y : Int32.t)
// C2PULSE-NEXT: requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
// C2PULSE-NEXT: returns res : Int32.t
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (Int32.add (square (triple (sum (doubleValue x) y))) (doubleValue (square (Int32.sub y x))));
// C2PULSE-NEXT: }
// C2PULSE-NEXT: fn conditionalProcessing
// C2PULSE-NEXT: (x : Int32.t)
// C2PULSE-NEXT: (y : Int32.t)
// C2PULSE-NEXT: requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
// C2PULSE-NEXT: returns res : Int32.t
// C2PULSE-NEXT: {
// C2PULSE-NEXT: if((Int32.eq (Int32.add ((Int32.mul x y)) 2l) 0l))
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (sum (triple x) (square (Int32.sub y x)));
// C2PULSE-NEXT: }
// C2PULSE-NEXT: else
// C2PULSE-NEXT: {
// C2PULSE-NEXT: (doubleValue (square (sum x y)));
// C2PULSE-NEXT: }
// C2PULSE: }

// C2PULSE: Success: Code transformed and syntax validated.

// PULSE: Verified module: Complex
// PULSE-NEXT: All verification conditions discharged successfully