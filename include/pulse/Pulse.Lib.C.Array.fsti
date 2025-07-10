module Pulse.Lib.C.Array
open Pulse
#lang-pulse

val pts_to_mask #t (arr: array t) (#[full_default()] f: perm) (v: erased (Seq.seq t)) (mask: nat -> prop) : slprop

ghost fn to_mask #t (arr: array t) #f #v
  requires arr |-> Frac f v
  ensures pts_to_mask arr #f v (fun _ -> True)

ghost fn from_mask #t (arr: array t) #f #v #mask
  requires pts_to_mask arr #f v mask
  requires pure (forall i. mask i)
  ensures arr |-> Frac f v

ghost fn split_mask #t (arr: array t) #f #v #mask (pred: nat -> prop)
  requires pts_to_mask arr #f v mask
  ensures pts_to_mask arr #f v (fun i -> mask i /\ pred i)
  ensures pts_to_mask arr #f v (fun i -> mask i /\ ~(pred i))

[@@allow_ambiguous]
ghost fn join_mask #t (arr: array t) #f #v #mask1 #mask2
  requires pts_to_mask arr #f v mask1
  requires pts_to_mask arr #f v mask2
  requires pure (forall i. ~(mask1 i /\ mask2 i))
  ensures pts_to_mask arr #f v (fun i -> mask1 i \/ mask2 i)

val array_base_t : Type0
val array_base #t (arr: array t) : GTot array_base_t
val array_offset #t (arr: array t) : GTot nat

let is_sub_array_of #t (sub arr: array t) (i j: nat) =
  pure (array_offset sub == array_offset arr + i
    /\  array_base sub == array_base arr
    /\  i + length sub == j)

unobservable
fn array_sub #t (arr: array t) #f #mask (i: SizeT.t) (j: SizeT.t)
    (#v: erased (Seq.seq t) { SizeT.v i <= SizeT.v j /\ SizeT.v j <= Seq.length (reveal v) })
  requires pts_to_mask arr #f v mask
  requires pure (forall k. SizeT.v i <= k /\ k < SizeT.v j ==> mask k)
  returns sub: array t
  ensures sub |-> Frac f (Seq.slice v (SizeT.v i) (SizeT.v j))
  ensures is_sub_array_of sub arr (SizeT.v i) (SizeT.v j)
  ensures pts_to_mask arr #f v (fun k -> mask k /\ ~(SizeT.v i <= k /\ k < SizeT.v j))

[@@allow_ambiguous]
ghost fn return_array_sub #t (arr sub: array t) #f (#v #vsub: erased (Seq.seq t)) #mask (#i #j: nat)
  requires pts_to_mask arr #f v mask
  requires sub |-> Frac f vsub
  requires is_sub_array_of sub arr i j
  requires pure (forall (k: nat). i <= k /\ k < j ==> ~(mask k))
  ensures exists* v'. pts_to_mask arr #f v' (fun k -> mask k \/ (i <= k /\ k < j))
    ** pure (Seq.length v == Seq.length v' /\ i + Seq.length vsub == j /\ j <= Seq.length v /\
      (forall (k: nat). k < Seq.length v' ==>
      Seq.index v' k == (if i <= k && k < j then Seq.index vsub (k - i) else Seq.index v k)))

val array_of_ref #t (r: ref t) : array t

let is_ref_to_elem #t (r: ref t) (arr: array t) (i: nat) =
  is_sub_array_of (array_of_ref r) arr i (i+1)

unobservable
fn array_at #t (arr: array t) #f #mask (i: SizeT.t) (#v: erased (Seq.seq t) { SizeT.v i < Seq.length (reveal v) })
  requires pts_to_mask arr #f v mask
  requires pure (mask (SizeT.v i))
  returns res: ref t
  ensures res |-> Frac f (Seq.index v (SizeT.v i))
  ensures is_ref_to_elem res arr (SizeT.v i)
  ensures pts_to_mask arr #f v (fun k -> mask k /\ k <> SizeT.v i)

[@@allow_ambiguous]
ghost fn return_array_at #t (arr: array t) (sub: ref t) #f (#v: erased (Seq.seq t)) (#vsub: erased t) #mask (#i: nat)
  requires pts_to_mask arr #f v mask
  requires sub |-> Frac f vsub
  requires is_ref_to_elem sub arr i
  requires pure (~(mask i))
  ensures exists* v'. pts_to_mask arr #f v' (fun k -> mask k \/ k == i)
    ** pure (i < Seq.length v /\ v' == Seq.upd v i vsub)