module Test_from_doc

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(a : Int32.t)
(b : bool)
requires pure (Int32.v a < 100)
returns Int32.t
{
let mut a : Int32.t = a;
let mut b : bool = b;
a := (Int32.add (! a) (bool_to_int32 (! b)));
let mut p: (ref Int32.t) = witness #_ #_;
p := (a);
(! p) := (Int32.add (! (! p)) 123l);
(! (! p));
}
