module Pulse_tutorial_loops

#lang-pulse

open Pulse
open Pulse.Lib.C



fn count_down
(x : ref Int32.t)
requires exists* v. (x |-> v) ** pure (as_int v >= 0)
ensures x |-> 0l
{
let mut keep_going = 1l;
while((Int32.eq (! keep_going) 1l);
)
invariant b. exists* k v. (keep_going |-> k) ** (x |-> v) ** pure (as_int v >= 0) ** pure (b==(k=1l)) ** pure (k<>1l ==> v==0l)
{
if((Int32.eq (! x) 0l))
{
keep_going := 0l;
}
else
{
x := (Int32.sub (! x) 1l);
};
};}

fn multiply_by_repeated_addition
(x : Int32.t)
(y : Int32.t)
requires pure (fits ( * ) (as_int x) (as_int y))
requires pure (as_int x >= 0)
returns i:int32
ensures pure (as_int i == as_int x * as_int y)
{
let mut ctr = 0l;
let mut acc = 0l;
while((Int32.lt (! ctr) x);
)
invariant b. exists* c a. (ctr |-> c) ** (acc |-> a) ** pure (as_int c <= as_int x) ** pure (as_int a == (as_int c * as_int y)) ** pure (b == (as_int c < as_int x))
{
ctr := (Int32.add (! ctr) 1l);
acc := (Int32.add (! acc) y);
};(! acc);
}
