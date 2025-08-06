module Pulse_tutorial_existentials

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn assign
(r : ( ref Int32.t) )
(v : Int32.t)
requires exists* s. r |-> s
ensures r |-> v
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = v;
(! r) := (! v);
}

fn make_even
(x : ( ref Int32.t) )
requires exists* s. (x |-> s) ** pure (fits ( * ) 2 (as_int s))
ensures exists* s. (x |-> s) ** pure (as_int s % 2 == 0)
{
let mut x : (ref Int32.t) = x;
(! x) := (Int32.add (! (! x)) (! (! x)));
}

[@@expect_failure]
fn make_even_explicit
(x : ( ref Int32.t) )
requires exists* w0. (x |-> w0) ** pure (fits ( * ) 2 (as_int w0))
ensures exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0)
{
let mut x : (ref Int32.t) = x;
with w0. assert (x |-> w0);
(! x) := (Int32.add (! (! x)) (! (! x)));
introduce exists* w1. (x |-> w1) ** pure (as_int w1 % 2 == 0) with (w0 +^ w0);
}

[@@expect_failure]
fn make_even_explicit_alt
(x : ( ref Int32.t) )
(y : ( ref Int32.t) )
requires exists* wx wy. (x |-> wx) ** (y |-> wy) ** pure (as_int wx % 2 == as_int wy % 2) ** pure (fits (+) (as_int wx) (as_int wy))
ensures exists* wx wy. (x |-> wx) ** (y |-> wy)
{
let mut x : (ref Int32.t) = x;
let mut y : (ref Int32.t) = y;
with wx wy. assert ((x |-> wx) ** (y |-> wy));
(! x) := (Int32.add (! (! x)) (! (! y)));
introduce exists* nx ny. (x |-> nx) ** (y |-> ny) ** pure (as_int nx % 2 == 0) with (wx +^ wy) wy;
}

fn call_make_even
(x : ( ref Int32.t) )
(#v:erased _)
requires x |-> v
requires pure (fits (+) (as_int v) (as_int v))
ensures exists* w. (x |-> w) ** pure (as_int w % 2 == 0)
{
let mut x : (ref Int32.t) = x;
(make_even (! x));
}
