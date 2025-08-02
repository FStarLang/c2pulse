module Pulse_tutorial_ref

#lang-pulse

open Pulse
open Pulse.Lib.C



fn value_of
(r : ( ref Int32.t) )
requires r |-> 'v
returns v:int32
ensures r |-> 'v
ensures pure (v == 'v)
{
let mut r : (ref Int32.t) = r;
(! (! r));
}

fn value_of_explicit
(r : ( ref Int32.t) )
(#w:erased _)
requires r |-> w
returns v:int32
ensures r |-> w
ensures pure (v == w)
{
let mut r : (ref Int32.t) = r;
(! (! r));
}

fn assign
(r : ( ref Int32.t) )
(v : Int32.t)
requires r |-> 'v
ensures r |-> v
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = v;
(! r) := (! v);
}

fn assign_alt
(r : ( ref Int32.t) )
(v : Int32.t)
requires exists* w. r |-> w
ensures r |-> v
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = v;
(! r) := (! v);
}

fn add
(r : ( ref Int32.t) )
(n : Int32.t)
(#w:erased _)
requires r |-> w
requires pure (fits (+) (as_int w) (as_int n))
ensures exists* ww. (r |-> ww) ** pure (as_int ww == as_int w + as_int n)
{
let mut r : (ref Int32.t) = r;
let mut n : Int32.t = n;
(! r) := (Int32.add (! (! r)) (! n));
}

fn add_alt
(r : ( ref Int32.t) )
(n : Int32.t)
(#w : erased _ { (fits (+) (as_int w) (as_int n)) })
requires r |-> w
ensures r |-> (w +^ n)
{
let mut r : (ref Int32.t) = r;
let mut n : Int32.t = n;
(! r) := (Int32.add (! (! r)) (! n));
}

fn quadruple
(r : ( ref Int32.t) )
(#w : erased _ { (fits ( * ) 4 (as_int w)) })
requires r |-> w
ensures exists* ww. (r |-> ww) ** pure (as_int ww == 4 * as_int w)
{
let mut r : (ref Int32.t) = r;
(add (! r) (! (! r)));
(add (! r) (! (! r)));
}

fn value_of_perm
(x : ( ref Int32.t) )
(#w:erased _)
(p:perm)
requires x |-> Frac p w
returns i:int32
ensures x |-> Frac p w
ensures pure (i == w)
{
let mut x : (ref Int32.t) = x;
(! (! x));
}

fn share_ref
(x : ( ref Int32.t) )
(#v:erased _)
(p:perm)
requires x |-> Frac p v
ensures x |-> Frac (p /. 2.0R) v
ensures x |-> Frac (p /. 2.0R) v
{
let mut x : (ref Int32.t) = x;
with vx. assert (x |-> vx);
share(vx);
}

fn gather_ref
(x : ( ref Int32.t) )
(#v0:erased _)
(#v1:erased _)
(p:perm)
requires x |-> Frac (p /. 2.0R) v0
requires x |-> Frac (p /. 2.0R) v1
ensures x |-> Frac p v0
ensures pure (v0 == v1)
{
let mut x : (ref Int32.t) = x;
with vx. assert (x |-> vx);
gather(vx);
}

fn max_perm
(x : ( ref Int32.t) )
(#v:erased _)
(p:perm)
requires x |-> Frac p v
requires pure (~(p <=. 1.0R))
ensures pure False
{
let mut x : (ref Int32.t) = x;
with vx. assert (x |-> vx);
pts_to_perm_bound vx;
unreachable();
}

fn alias_ref
(r : ( ref Int32.t) )
(#v:erased _)
(p:perm)
requires r |-> Frac p v
returns s: ref int32
ensures s |-> Frac (p /. 2.0R) v
ensures s |-> Frac (p /. 2.0R) v
ensures pure (s == r)
{
let mut r : (ref Int32.t) = r;
with vr. assert (r |-> vr);
share vr;
(! r);
}

fn incr
(r : ( ref Int32.t) )
(#vr:erased _)
requires r |-> vr
requires pure (fits (+) (as_int vr) 1)
ensures exists* w. (r |-> w) ** pure (as_int w == as_int vr + 1)
{
let mut r : (ref Int32.t) = r;
(! r) := (Int32.add (! (! r)) 1l);
}

fn one ()
requires emp
returns i:int32
ensures pure (as_int i == 1)
{
let mut i : Int32.t = 0l;
(incr (i));
(! i);
}

[@@expect_failure [228]]
fn refs_are_scoped ()
returns s:ref int32
ensures s |-> 0l
{
let mut s : Int32.t = 0l;
(s);
}
