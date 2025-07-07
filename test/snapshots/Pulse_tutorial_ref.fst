module Pulse_tutorial_ref

#lang-pulse

open Pulse
open Pulse.Lib.C



fn value_of
(r : ref Int32.t)
requires r |-> 'v
returns v:int32
ensures r |-> 'v
ensures pure (v == 'v)
{
(! r);
}

fn value_of_explicit
(r : ref Int32.t)
(#w:erased _)
requires r |-> w
returns v:int32
ensures r |-> w
ensures pure (v == w)
{
(! r);
}

fn assign
(r : ref Int32.t)
(v : Int32.t)
requires r |-> 'v
ensures r |-> v
{
r := v;
}

fn assign_alt
(r : ref Int32.t)
(v : Int32.t)
requires exists* w. r |-> w
ensures r |-> v
{
r := v;
}

fn add
(r : ref Int32.t)
(n : Int32.t)
(#w:erased _)
requires r |-> w
requires pure (fits (+) (as_int w) (as_int n))
ensures exists* ww. (r |-> ww) ** pure (as_int ww == as_int w + as_int n)
{
r := (Int32.add (! r) n);
}

fn add_alt
(r : ref Int32.t)
(n : Int32.t)
(#w : erased _ { (fits (+) (as_int w) (as_int n)) })
requires r |-> w
ensures r |-> (w +^ n)
{
r := (Int32.add (! r) n);
}

fn quadruple
(r : ref Int32.t)
(#w : erased _ { (fits ( * ) 4 (as_int w)) })
requires r |-> w
ensures exists* ww. (r |-> ww) ** pure (as_int ww == 4 * as_int w)
{
(add r (! r));
(add r (! r));
}

fn value_of_perm
(x : ref Int32.t)
(#w:erased _)
(p:perm)
requires x |-> Frac p w
returns i:int32
ensures x |-> Frac p w
ensures pure (i == w)
{
(! x);
}

fn share_ref
(x : ref Int32.t)
(#v:erased _)
(p:perm)
requires x |-> Frac p v
ensures x |-> Frac (p /. 2.0R) v
ensures x |-> Frac (p /. 2.0R) v
{
share(x);
}

fn gather_ref
(x : ref Int32.t)
(#v0:erased _)
(#v1:erased _)
(p:perm)
requires x |-> Frac (p /. 2.0R) v0
requires x |-> Frac (p /. 2.0R) v1
ensures x |-> Frac p v0
ensures pure (v0 == v1)
{
gather(x);
}

fn max_perm
(x : ref Int32.t)
(#v:erased _)
(p:perm)
requires x |-> Frac p v
requires pure (~(p <=. 1.0R))
ensures pure False
{
pts_to_perm_bound x;
unreachable();
}

fn alias_ref
(r : ref Int32.t)
(#v:erased _)
(p:perm)
requires r |-> Frac p v
returns s: ref int32
ensures s |-> Frac (p /. 2.0R) v
ensures s |-> Frac (p /. 2.0R) v
ensures pure (s == r)
{
share r;
r;
}

fn incr
(r : ref Int32.t)
(#vr:erased _)
requires r |-> vr
requires pure (fits (+) (as_int vr) 1)
ensures exists* w. (r |-> w) ** pure (as_int w == as_int vr + 1)
{
r := (Int32.add (! r) 1l);
}

fn one ()
requires emp
returns i:int32
ensures pure (as_int i == 1)
{
let mut i = 0l;
(incr i);
(! i);
}
