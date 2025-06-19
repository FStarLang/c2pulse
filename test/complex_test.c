// RUN: %c2pulse %s 2>&1 | %{FILECHECK} %s

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

// CHECK: fn triple
// CHECK-NEXT: (x : Int32.t)
// CHECK-NEXT: requires pure (Int32.fits (Int32.v x * 3))
// CHECK-NEXT: returns res : Int32.t
// CHECK-NEXT: ensures pure (Int32.v x * 3 == Int32.v res)
// CHECK-NEXT: {
// CHECK-NEXT: (Int32.mul x 3l);
// CHECK-NEXT: }
// CHECK-NEXT: fn square
// CHECK-NEXT: (x : Int32.t)
// CHECK-NEXT: requires pure (Int32.fits (Int32.v x * Int32.v x))
// CHECK-NEXT: returns res : Int32.t
// CHECK-NEXT: ensures pure (Int32.v x * Int32.v x == Int32.v res)
// CHECK-NEXT: {
// CHECK-NEXT: (Int32.mul x x);
// CHECK-NEXT: }
// CHECK-NEXT: fn doubleValue
// CHECK-NEXT: (x : Int32.t)
// CHECK-NEXT: requires pure (Int32.fits (Int32.v x * 2))
// CHECK-NEXT: returns res : Int32.t
// CHECK-NEXT: ensures pure (Int32.v x * 2 == Int32.v res)
// CHECK-NEXT: {
// CHECK-NEXT: (Int32.mul x 2l);
// CHECK-NEXT: }
// CHECK-NEXT: fn sum
// CHECK-NEXT: (x : Int32.t)
// CHECK-NEXT: (y : Int32.t)
// CHECK-NEXT: requires pure (Int32.fits (Int32.v x + Int32.v y))
// CHECK-NEXT: returns res : Int32.t
// CHECK-NEXT: ensures pure (Int32.v x + Int32.v y == Int32.v res)
// CHECK-NEXT: {
// CHECK-NEXT: (Int32.add x y);
// CHECK-NEXT: }
// CHECK-NEXT: fn rec recursiveFunction
// CHECK-NEXT: (x : Int32.t)
// CHECK-NEXT: (limit : Int32.t)
// CHECK-NEXT: requires pure False
// CHECK-NEXT: returns res : Int32.t
// CHECK-NEXT: {
// CHECK-NEXT: if((Int32.eq x limit))
// CHECK-NEXT: {
// CHECK-NEXT: x;
// CHECK-NEXT: }
// CHECK-NEXT: else
// CHECK-NEXT: {
// CHECK-NEXT: (sum 12l (recursiveFunction (Int32.add x 1l) limit));
// CHECK-NEXT: }
// CHECK-NEXT: }
// CHECK-NEXT: fn complexComputation
// CHECK-NEXT: (x : Int32.t)
// CHECK-NEXT: (y : Int32.t)
// CHECK-NEXT: requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
// CHECK-NEXT: returns res : Int32.t
// CHECK-NEXT: {
// CHECK-NEXT: (Int32.add (square (triple (sum (doubleValue x) y))) (doubleValue (square (Int32.sub y x))));
// CHECK-NEXT: }
// CHECK-NEXT: fn conditionalProcessing
// CHECK-NEXT: (x : Int32.t)
// CHECK-NEXT: (y : Int32.t)
// CHECK-NEXT: requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
// CHECK-NEXT: returns res : Int32.t
// CHECK-NEXT: {
// CHECK-NEXT: if((Int32.eq (Int32.add ((Int32.mul x y)) 2l) 0l))
// CHECK-NEXT: {
// CHECK-NEXT: (sum (triple x) (square (Int32.sub y x)));
// CHECK-NEXT: }
// CHECK-NEXT: else
// CHECK-NEXT: {
// CHECK-NEXT: (doubleValue (square (sum x y)));
// CHECK-NEXT: }
// CHECK: }

// CHECK: Success: Code transformed and syntax validated.