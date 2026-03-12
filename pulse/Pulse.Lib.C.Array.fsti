module Pulse.Lib.C.Array
open Pulse
open Pulse.Lib.C.Inhabited
module A = Pulse.Lib.Array
module SZ = FStar.SizeT

#lang-pulse

let array_value_of #a (x: array a) #p #m #y =
  observe (fun y -> pts_to_mask x #p y m) #y

let array_full_mask #t (s: Seq.seq (option t)) (mask: nat -> prop) =
  forall (i: nat). i < Seq.length s ==> mask i

let array_initialized #t (s: Seq.seq (option t)) : prop =
  forall (i: nat). i < Seq.length s ==> Some? (Seq.index s i)

[@@pulse_eager_unfold]
let array_pts_to_uninit (#t: Type u#a) (a: array t) (s: Seq.seq (option t)) (mask: nat -> prop) =
  pts_to_mask a s mask ** with_pure (array_full_mask s mask) fun _ -> emp

[@@pulse_eager_unfold]
let array_pts_to (#t: Type u#a) ([@@@mkey] a: array t) (p: perm) (s: Seq.seq (option t)) (mask: nat -> prop) : slprop =
  pts_to_mask a #p s mask ** with_pure (array_full_mask s mask /\ array_initialized s) fun _ -> emp

val freeable_array (#a:Type) (r:array a) : slprop

fn alloc_array u#a (#a:Type u#a) (sz:SizeT.t)
  returns r : array a
  ensures freeable_array r
  ensures exists* s mask. array_pts_to_uninit r s mask
  ensures pure (Seq.length (array_value_of r) == (SizeT.v sz))

fn free_array u#a (#a:Type u#a) (r:array a)
  requires exists* s mask. array_pts_to_uninit r s mask
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
  ensures exists* (s: Seq.seq (option a)) mask.
    pts_to_mask r s mask **
    pure (Seq.length s == SZ.v sz /\ array_initialized s /\ array_full_mask s mask /\
      (forall (i: nat). i < Seq.length s ==> Seq.index s i == Some zero_default))
{
  let r = calloc_array #a sz;
  arr_to_mask r;
  r
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
  exists* s mask. array_pts_to a 1.0R s mask

// Array read that works in spec contexts (has rewrites_to).
fn array_read u#a (#t: Type u#a) (a: array t) (i: SZ.t)
  (#p: perm)
  (#s: Ghost.erased (Seq.seq (option t)) { SZ.v i < Seq.length s })
  #mask
  preserves pts_to_mask a #p s mask
  requires with_pure (mask (SZ.v i) /\ Some? (Seq.index s (SZ.v i)))
  returns res: t
  ensures rewrites_to res (Some?.v (Seq.index s (SZ.v i)))
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

// ---------------------------------------------------------------------------
// ArrayPtr: pointers into arrays (zero-length sub-arrays sharing a base)
//
// An arrayptr is just a Pulse array with length 0 and the same base as its
// parent. arrayptr_pts_to is a pure proposition asserting this relationship.
// ---------------------------------------------------------------------------

/// Offset of x relative to y (may be negative).
private let arrayptr_off (#t: Type) (x y: array t) : GTot int =
  offset_of x - offset_of y

/// Predicate asserting that arrayptr `x` points into array `y`.
/// Same base, zero length.
let arrayptr_pts_to (#t: Type u#a) ([@@@mkey] x: array t) (y: array t) : slprop =
  pure (base_of x == base_of y /\ length x == 0)

let arrayptr_parent #a (x: array a) #y =
  observe (arrayptr_pts_to x) #y

ghost fn arrayptr_pts_to_dup u#a #t x y : duplicable_f (arrayptr_pts_to u#a #t x y) = {
  unfold arrayptr_pts_to x y;
  fold arrayptr_pts_to x y;
  fold arrayptr_pts_to x y;
}

instance duplicable_arrayptr_pts_to #t x y : duplicable (arrayptr_pts_to x y) =
  { dup_f = fun _ -> arrayptr_pts_to_dup x y }

/// Create an arrayptr from an array at offset `i`.
val array_to_arrayptr (#t: Type u#a) (arr: array t) (i: SZ.t)
  : stt (array t)
    emp
    (fun r -> arrayptr_pts_to r arr ** pure (offset_of r == offset_of arr + SZ.v i))

/// Shift an arrayptr by `n` positions.
val arrayptr_shift (#t: Type u#a) (x: array t) (n: SZ.t) (#y: erased (array t))
  : stt (array t)
    (arrayptr_pts_to x y)
    (fun r -> arrayptr_pts_to x y ** arrayptr_pts_to r y **
      pure (offset_of r == offset_of x + SZ.v n))

/// Read through an arrayptr at index `i`, borrowing permissions from parent `y`.
val arrayptr_read (#t: Type u#a) (x: array t) (i: SZ.t)
  (#y: erased (array t))
  (#p: perm) (#s: Ghost.erased (Seq.seq (option t))) (#mask: Ghost.erased (nat -> prop))
  : stt t
    (arrayptr_pts_to x y ** pts_to_mask y #p s mask **
      pure (0 <= arrayptr_off x y + SZ.v i /\
            arrayptr_off x y + SZ.v i < Seq.length s /\
            reveal mask (arrayptr_off x y + SZ.v i) /\
            Some? (Seq.index s (arrayptr_off x y + SZ.v i))))
    (fun res -> arrayptr_pts_to x y ** pts_to_mask y #p s mask)

/// Write through an arrayptr at index `i`, using permissions from parent `y`.
val arrayptr_write (#t: Type u#a)
  (x: array t) (i: SZ.t) (v: t)
  (#y: array t)
  (#s: Ghost.erased (Seq.seq (option t))) (#mask: Ghost.erased (nat -> prop))
  : stt unit
    (arrayptr_pts_to x y ** pts_to_mask y s mask **
      pure (0 <= arrayptr_off x y + SZ.v i /\
            arrayptr_off x y + SZ.v i < Seq.length s /\
            reveal mask (arrayptr_off x y + SZ.v i)))
    (fun _ ->
      exists* s'. arrayptr_pts_to x y **
        pts_to_mask y s' mask **
        pure (0 <= arrayptr_off x y + SZ.v i /\
              arrayptr_off x y + SZ.v i < Seq.length s /\
              s' == Seq.upd s (arrayptr_off x y + SZ.v i) (Some v)))

/// Subtract two arrayptrs to get their offset difference.
val arrayptr_diff (#t: Type) (x z: array t)
  : (r:Pulse.Lib.C.PtrdiffT.t{Pulse.Lib.C.PtrdiffT.v r == offset_of x - offset_of z})

/// Compare two arrayptrs for equality.
val arrayptr_eq (#t: Type) (x z: array t) : (r:bool{r == (offset_of x = offset_of z)})

/// Check if arrayptr x offset is <= z offset.
val arrayptr_lte (#t: Type) (x z: array t) : (r:bool{r == (offset_of x <= offset_of z)})

/// Check if arrayptr x offset is < z offset.
val arrayptr_lt (#t: Type) (x z: array t) : (r:bool{r == (offset_of x < offset_of z)})

/// Drop an arrayptr_pts_to predicate (for scope exit / cleanup).
val arrayptr_drop (#t: Type u#a) (x: array t) (#y: array t)
  : stt_ghost unit emp_inames
    (arrayptr_pts_to x y)
    (fun _ -> emp)