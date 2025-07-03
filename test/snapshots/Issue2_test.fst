module Issue2_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn value_of_explicit
(r : ref Int32.t)
(#w:_)
requires r |-> w
returns v:Int32.t
ensures r |-> w
ensures pure (v == w)
{
(! r);
}
