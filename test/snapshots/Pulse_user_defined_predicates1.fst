module Pulse_user_defined_predicates1

#lang-pulse

open Pulse
open Pulse.Lib.C



let pts_to_diag (#a:_) (r0 r1:ref a) (v:a) : slprop = (r0 |-> v) ** (r1 |-> v)
fn double_int
(r0 : ref Int32.t)
(r1 : ref Int32.t)
(#v:erased _ { fits ( * ) 2 (as_int v) })
requires pts_to_diag r0 r1 v
ensures exists* w. pts_to_diag r0 r1 w ** pure (as_int w = 2 * as_int v)
{
unfold (pts_to_diag r0 r1 v);
r0 := (Int32.add (! r0) (! r1));
r1 := (! r0);
fold (pts_to_diag r0 r1);
}
