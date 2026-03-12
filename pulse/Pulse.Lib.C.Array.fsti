module Pulse.Lib.C.Array
open Pulse
open Pulse.Lib.C.Inhabited
module A = Pulse.Lib.Array
module SZ = FStar.SizeT

#lang-pulse

[@@pulse_eager_unfold]
let array_pts_to_uninit (#t: Type u#a) (a: array t) (len: nat) =
  exists* s. pts_to_mask a s (fun _ -> True) ** pure (Seq.length s == len)

[@@pulse_eager_unfold]
let array_pts_to (#t: Type u#a) ([@@@mkey] a: array t) (p: perm) (s: Seq.seq (option t)) : slprop =
  exists* mask. pts_to_mask a #p s mask
    ** pure (forall (i: nat). i < Seq.length s ==> Some? (Seq.index s i) /\ mask i)

val freeable_array (#a:Type) (r:array a) : slprop

fn alloc_array u#a (#a:Type u#a) (sz:SizeT.t)
  returns r : array a
  ensures freeable_array r
  ensures array_pts_to_uninit r (SizeT.v sz)

let free_array_pre (#a: Type u#a) (r: array a) : slprop =
  A.pts_to_uninit_post r

[@@pulse_intro]
ghost fn intro_free_array_pre_init u#a (#a: Type u#a) (r: array a)
  requires live r
  ensures free_array_pre r
{
  A.to_mask r;
  fold free_array_pre r;
}

[@@pulse_intro]
ghost fn intro_free_array_pre_uninit u#a (#a: Type u#a) (r: array a)
  requires A.pts_to_uninit_post r
  ensures free_array_pre r
{
  fold free_array_pre r;
}

fn free_array u#a (#a:Type u#a) (r:array a)
  requires free_array_pre r
  requires freeable_array r

fn calloc_array u#a (#a:Type u#a) {| has_zero_default a |} (sz:SizeT.t)
  returns r : array a
  ensures freeable_array r
  ensures A.pts_to r (Seq.create (SizeT.v sz) zero_default)

// Convert pts_to to pts_to_mask for uniform array handling
ghost fn arr_to_mask u#a (#t: Type u#a) (arr: array t) (#p: perm) (#v: erased (Seq.seq t))
  requires A.pts_to arr #p v
  ensures exists* (s: Seq.seq (option t)). pts_to_mask arr #p s (fun _ -> True)
    ** pure (Seq.length s == Seq.length v /\
      (forall (i: nat). i < Seq.length s ==> Seq.index s i == Some (Seq.index v i)))
{
  A.to_mask arr;
}

// Calloc that returns pts_to_mask instead of pts_to
fn calloc_array_mask u#a (#a:Type u#a) {| has_zero_default a |} (sz:SizeT.t)
  returns r : array a
  ensures freeable_array r
  ensures exists* (s: Seq.seq (option a)).
    pts_to_mask r s (fun _ -> True) **
    pure (Seq.length s == SZ.v sz /\
      (forall (i: nat). i < Seq.length s ==> Seq.index s i == Some zero_default))
{
  let r = calloc_array #a sz;
  arr_to_mask r;
  r
}

// Free from pts_to_mask context
[@@pulse_intro]
ghost fn intro_free_array_pre_from_mask u#a (#a: Type u#a) (r: array a) #mask
  requires pts_to_mask r #1.0R 'y mask
  requires pure (forall (i: nat). i < Seq.length 'y ==> mask i)
  ensures free_array_pre r
{
  fold free_array_pre r;
}

let has_length #a ([@@@mkey] x: a) (len: nat) : slprop = emp

[@@pulse_intro]
ghost fn intro_has_length_init u#a (#a: Type u#a) (x: array a)
  requires pts_to x #'p 'y
  ensures has_length x (Seq.length 'y)
{
  drop_ (pts_to x #'p 'y);
  fold has_length x (Seq.length 'y);
}

[@@pulse_intro]
ghost fn intro_has_length_mask u#a (#a: Type u#a) (x: array a)
  requires A.pts_to_mask x #'p 'y 'mask
  ensures has_length x (Seq.length 'y)
{
  drop_ (A.pts_to_mask x #'p _ _);
  fold has_length x (Seq.length 'y);
}

let length_of #a (x: a) #y = observe (has_length x) #y

// live_array: array resource preserved across loop iterations
[@@pulse_eager_unfold]
let live_array (#t: Type u#a) (a: array t) : slprop =
  exists* (s: Seq.seq (option t)) mask. pts_to_mask a s mask
    ** pure (forall (i: nat). i < Seq.length s ==> Some? (Seq.index s i) /\ mask i)

let some_v (#a: Type) {| has_zero_default a |} (x: option a) : a =
  match x with
  | Some v -> v
  | None -> zero_default

// Array read that works in spec contexts (has rewrites_to).
fn array_read u#a (#t: Type u#a) {| has_zero_default t |} (a: array t) (i: SZ.t)
  (#p: perm)
  (#s: Ghost.erased (Seq.seq (option t)) { SZ.v i < Seq.length s })
  #mask
  requires pts_to_mask a #p s mask ** pure (mask (SZ.v i) /\ Some? (Seq.index s (SZ.v i)))
  returns res: t
  ensures pts_to_mask a #p s mask ** rewrites_to res (some_v (Seq.index s (SZ.v i)))
{
  A.mask_read a i
}

// Array write for use with pts_to_mask.
inline_for_extraction
fn array_write u#a (#t: Type u#a) (a: array t) (i: SZ.t) (v: t)
  (#s: Ghost.erased (Seq.seq (option t)) { SZ.v i < Seq.length s })
  #mask
  requires pts_to_mask a s mask
  requires pure (mask (SZ.v i))
  ensures pts_to_mask a (Seq.upd s (SZ.v i) (Some v)) mask
{
  A.mask_write a i v
}