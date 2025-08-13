module Pulse.Lib.C.Ref
#lang-pulse
open Pulse
open Pulse.Lib.C.Inhabited
include Pulse.Lib.Reference
open FStar.Tactics.Typeclasses { noinst }

instance inhabited_ref (a:Type) : Pulse.Lib.C.Inhabited.inhabited (ref a) = {
  witness = null
}

instance inhabited_array (a:Type) : Pulse.Lib.C.Inhabited.inhabited (array a) = {
  witness = magic(); // update to Array.null once it's added
}


val freeable (#a:Type) ([@@@mkey] r:ref a) : slprop

fn alloc_ref (#a:Type) {| inhabited a |} ()
  returns  r : ref a
  ensures  exists* x. r |-> x
  ensures  freeable r

fn free_ref (#a:Type) (r:ref a)
  requires freeable r
  requires exists* x. r |-> x

// let live (#a:Type) ([@@@mkey] r:ref a) : slprop =
//   exists* x. r |-> x

let maybe_live (#a:Type) ([@@@mkey] r:ref a) : slprop =
  if is_null r then emp
  else live r

let maybe_pts_to (#a:Type) ([@@@mkey] r:ref a) (#f:perm) (x:option a) : slprop =
  if is_null r then pure (x == None)
  else exists* v. (r |-> Frac f v) ** pure (x == Some v)


[@@FStar.Tactics.Typeclasses.fundeps [1]; // The pointer determines the representation
   pulse_unfold]
class has_pts_to_nullable (p r : Type) = {
  [@@@pulse_unfold]
  pts_to_nullable : p -> (#[full_default()] f : perm) -> r -> slprop;
}

(* Always full permission *)
[@@pulse_unfold]
let ( |->? ) #p #r {| has_pts_to_nullable p r |} = pts_to_nullable #p #r

[@@pulse_unfold; noinst]
instance pts_to_nullable_frac (p a : Type) (d : has_pts_to_nullable p a) : has_pts_to_nullable p (frac a) = {
  pts_to_nullable = (fun p #f' (Frac f v) -> d.pts_to_nullable p #(f' *. f) v);
}

[@@pulse_unfold]
instance pts_to_nullable_pts_to (a:Type) : has_pts_to_nullable (ref a) (option a) = {
  pts_to_nullable = (fun r #f v -> maybe_pts_to r #f v);
}

ghost
fn elim_null (#a:Type) (r:ref a) #p (#x:option a)
requires r |->? Frac p x
requires pure (is_null r)
ensures pure (x == None)
{
  rewrite (r |->? Frac p x) as maybe_pts_to r #p x;
  unfold maybe_pts_to;
  rewrite each (is_null r) as true;
}

ghost
fn elim_non_null (#a:Type) (r:ref a) #p (#x:option a)
requires r |->? Frac p x
requires pure (not (is_null r))
ensures exists* (v:a). (r |-> Frac p v) **pure (x == Some v)
{
  rewrite (r |->? Frac p x) as maybe_pts_to r #p x;
  unfold maybe_pts_to;
  rewrite each (is_null r) as false;
}

ghost
fn intro_non_null (#a:Type) (r:ref a) #p (#x:a)
requires r |-> Frac p x
ensures r |->? Frac p (Some x)
{
  pts_to_not_null r;
  introduce exists* v. (r |-> Frac p v) ** pure (Some x == Some v)
  with x;
  rewrite (exists* (v:a). (r |-> Frac p v) ** pure (Some x == Some v)) as (maybe_pts_to r #p (Some x));
  rewrite (maybe_pts_to r #p (Some x)) as (r |->? Frac p (Some x));
}

ghost
fn intro_null (#a:Type) (r:ref a) #p
requires pure (is_null r)
ensures r |->? Frac p None
{
  rewrite (pure (None #a == None #a)) as (maybe_pts_to r #p None);
  rewrite (maybe_pts_to r #p None) as (r |->? Frac p None);
}


ghost
fn elim_intro_null (#a:Type) (r:ref a) #p (#x:option a)
requires r |->? Frac p x
requires pure (is_null r)
ensures (r |->? Frac p x) ** pure (x == None)
{
  elim_null r;
  intro_null #a r #p;
}



let _test0 (x:ref int) (y:int) = x |-> y
let _test1 (x:ref int) (y:option int) = x |->? y
let _test2 (x:ref int) (y:int) = x |-> Frac 0.170R y
let _test3 (x:ref int) (y:int) = x |->? Frac 0.42R (Some y)
let _test4 (x:ref int) = x |->? Frac 0.42R None
