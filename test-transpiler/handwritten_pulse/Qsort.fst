module Qsort
open Pulse
open Pulse.Lib.C
#lang-pulse

fn swap (a b : ref Int32.t)
  requires a |-> 'va
  requires b |-> 'vb
  ensures b |-> 'va
  ensures a |-> 'vb
{
  let tmp = !a;
  a := !b;
  b := tmp;
}

assume val pts_to_mask #t (arr: array t) (#[full_default()] f: perm) (v: erased (Seq.seq t)) (mask: nat -> prop) : slprop

ghost fn to_mask #t (arr: array t) #f #v
  requires arr |-> Frac f v
  ensures pts_to_mask arr #f v (fun _ -> True)
{ admit () }

ghost fn from_mask #t (arr: array t) #f #v #mask
  requires pts_to_mask arr #f v mask
  requires pure (forall i. mask i)
  ensures arr |-> Frac f v
{ admit () }

ghost fn split_mask #t (arr: array t) #f #v #mask (pred: nat -> prop)
  requires pts_to_mask arr #f v mask
  ensures pts_to_mask arr #f v (fun i -> mask i /\ pred i)
  ensures pts_to_mask arr #f v (fun i -> mask i /\ ~(pred i))
{ admit () }

[@@allow_ambiguous]
ghost fn join_mask #t (arr: array t) #f #v #mask1 #mask2
  requires pts_to_mask arr #f v mask1
  requires pts_to_mask arr #f v mask2
  requires pure (forall i. ~(mask1 i /\ mask2 i))
  ensures pts_to_mask arr #f v (fun i -> mask1 i \/ mask2 i)
{ admit () }

assume val array_base_t : Type0
assume val array_base #t (arr: array t) : GTot array_base_t
assume val array_offset #t (arr: array t) : GTot nat

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
{ admit () }

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
{ admit () }

assume val array_of_ref #t (r: ref t) : array t

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
{ admit () }

[@@allow_ambiguous]
ghost fn return_array_at #t (arr: array t) (sub: ref t) #f (#v: erased (Seq.seq t)) (#vsub: erased t) #mask (#i: nat)
  requires pts_to_mask arr #f v mask
  requires sub |-> Frac f vsub
  requires is_ref_to_elem sub arr i
  requires pure (~(mask i))
  ensures exists* v'. pts_to_mask arr #f v' (fun k -> mask k \/ k == i)
    ** pure (i < Seq.length v /\ v' == Seq.upd v i vsub)
{ admit () }

fn qsort_partition (arr : array Int32.t) (len : SizeT.t)
  requires arr |-> 'varr
  requires pure (Seq.length 'varr == SizeT.v len)
  requires pure (SizeT.v len > 1)
  returns pivot_idx: SizeT.t
  ensures pure (SizeT.v pivot_idx < Seq.length 'varr)
  ensures exists* varr'.  (arr |-> varr') **
    pure (Seq.length 'varr == Seq.length varr' /\ SizeT.v pivot_idx < Seq.length varr' /\
      (let pivot = Seq.index varr' (SizeT.v pivot_idx) in
        forall (k: nat). k < Seq.length varr' ==>
          (k < SizeT.v pivot_idx ==> Seq.index varr' k `Int32.lte` pivot) /\
          (SizeT.v pivot_idx < k ==> Seq.index varr' k `Int32.gte` pivot)))
{
  let pivot = arr.(0sz);
  let mut i: SizeT.t = 1sz;
  let mut j: SizeT.t = len;
  while ((!i `SizeT.lt` !j))
    invariant b.  exists* varr vi vj.  (arr |-> varr) ** (i |-> vi) ** (j |-> vj) **
      pure (
        b == SizeT.lt vi vj /\
        Seq.length varr == SizeT.v len /\
        0 < SizeT.v vi /\ SizeT.v vi <= SizeT.v vj /\ SizeT.v vj <= SizeT.v len /\
        Seq.index varr 0 == pivot /\
        (forall (k: nat). k < SizeT.v vi ==> Int32.lte (Seq.index varr k) pivot) /\
        (forall (k: nat). SizeT.v vj <= k /\ k < Seq.length varr ==> Int32.lte pivot (Seq.index varr k))
      )
  {
    if (arr.(!i) `Int32.lte` pivot) {
      i := SizeT.add (!i) 1sz;
    } else if (arr.(!j `SizeT.sub` 1sz) `Int32.gte` pivot) {
      j := SizeT.sub (!j) 1sz;
    } else {
      j := SizeT.sub (!j) 1sz;
      to_mask arr;
      swap (array_at arr (!i)) (array_at arr (!j));
      return_array_at arr _;
      return_array_at arr _;
      from_mask arr;
      i := SizeT.add (!i) 1sz;
    }
  };
  if (!i `SizeT.gt` 1sz) {
    i := !i `SizeT.sub` 1sz;
    to_mask arr;
    swap (array_at arr 0sz) (array_at arr (!i));
    return_array_at arr _;
    return_array_at arr _;
    from_mask arr;
    !i
  } else {
    0sz
  }
}

fn rec qsort (arr: array Int32.t) (len: SizeT.t)
  requires arr |-> 'varr
  requires pure (Seq.length 'varr == SizeT.v len)
  ensures exists* varr'. (arr |-> varr') **
    pure (Seq.length 'varr == Seq.length varr')
{
  if (SizeT.v len > 1) {
    let pivot_idx = qsort_partition arr len;
    with varr1. assert (arr |-> varr1);
    to_mask arr;
    assert pure (SizeT.v pivot_idx < Seq.length varr1);
    qsort (array_sub arr 0sz pivot_idx) pivot_idx; return_array_sub arr _;
    qsort (array_sub arr (SizeT.add pivot_idx 1sz) len)
      (SizeT.sub len (SizeT.add pivot_idx 1sz)); return_array_sub arr _;
    from_mask arr;
  }
}