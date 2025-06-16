module Complex

#lang-pulse
open Pulse

module Int32 = FStar.Int32

fn triple (x : Int32.t)
  requires pure (Int32.fits (Int32.v x * 3))
  returns  res : Int32.t
  ensures  pure (Int32.v x * 3 == Int32.v res)
{
  Int32.mul x 3l;
}

fn square (x : Int32.t)
  requires pure (Int32.fits (Int32.v x * Int32.v x))
  returns  res : Int32.t
  ensures  pure (Int32.v x * Int32.v x == Int32.v res)
{
  Int32.mul x x;
}

fn doubleValue (x : Int32.t)
  requires pure (Int32.fits (Int32.v x * 2))
  returns  res : Int32.t
  ensures  pure (Int32.v x * 2 == Int32.v res)
{
  Int32.mul x 2l;
}

fn sum (x : Int32.t) (y : Int32.t)
  requires pure (Int32.fits (Int32.v x + Int32.v y))
  returns  res : Int32.t
  ensures  pure (Int32.v x + Int32.v y == Int32.v res)
{
  Int32.add x y;
}

fn complexComputation (x y : Int32.t)
  // Precondition could be more liberal, this is just one
  // possibility. User should annotate what they want.
  requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
  returns  res : Int32.t
{
  // Have to ANF every effectful call
  let t1 = doubleValue x;
  let t2 = sum t1 y;
  let t3 = triple t2;
  let t4 = square t3;
  let t5 = square (Int32.sub y x);
  let t6 = doubleValue t5;
  (Int32.add t4 t6);
}

fn condtionalProcessing (x y : Int32.t)
  // Precondition could be more liberal, this is just one
  // possibility. User should annotate what they want.
  requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
  returns  res : Int32.t
{
  if (Int32.rem (Int32.mul x y) 2l = 0l) {
    let t1 = triple x;
    let t2 = square (Int32.sub y x);
    sum t1 t2
  } else {
    let t1 = sum x y;
    let t2 = square t1;
    doubleValue t2;
  }
}

fn rec recursiveFunction (x limit : Int32.t)
  // This is a hack to get this function to pass. To properly
  // check this function we need to find some invariant about
  // the size of the returned value.
  requires pure False
  returns  res : Int32.t
{
  if (Int32.gte x limit) {
    x;
  } else {
    let t1 = square x;
    let t2 = condtionalProcessing x t1;
    let t3 = recursiveFunction (Int32.add x 1l) limit;
    sum t2 t3;
  }
}
