module Complex_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn triple
(x : Int32.t)
requires pure (Int32.fits (Int32.v x * 3))
returns res : Int32.t
ensures pure (Int32.v x * 3 == Int32.v res)
{
(Int32.mul x 3l);
}

fn square
(x : Int32.t)
requires pure (Int32.fits (Int32.v x * Int32.v x))
returns res : Int32.t
ensures pure (Int32.v x * Int32.v x == Int32.v res)
{
(Int32.mul x x);
}

fn doubleValue
(x : Int32.t)
requires pure (Int32.fits (Int32.v x * 2))
returns res : Int32.t
ensures pure (Int32.v x * 2 == Int32.v res)
{
(Int32.mul x 2l);
}

fn sum
(x : Int32.t)
(y : Int32.t)
requires pure (Int32.fits (Int32.v x + Int32.v y))
returns res : Int32.t
ensures pure (Int32.v x + Int32.v y == Int32.v res)
{
(Int32.add x y);
}

fn rec recursiveFunction
(x : Int32.t)
(limit : Int32.t)
requires pure False
returns res : Int32.t
{
if((Int32.gte x limit))
{
x;
}
else
{
(sum 12l (recursiveFunction (Int32.add x 1l) limit));
};
}

fn complexComputation
(x : Int32.t)
(y : Int32.t)
requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
returns res : Int32.t
{
(Int32.add (square (triple (sum (doubleValue x) y))) (doubleValue (square (Int32.sub y x))));
}

fn conditionalProcessing
(x : Int32.t)
(y : Int32.t)
requires pure (abs (Int32.v x) < 1000 /\ abs (Int32.v y) < 1000)
returns res : Int32.t
{
if((Int32.eq (Int32.rem ((Int32.mul x y)) 2l) 0l))
{
(sum (triple x) (square (Int32.sub y x)));
}
else
{
(doubleValue (square (sum x y)));
};
}
