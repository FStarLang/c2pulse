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
    to_mask arr;
    qsort (array_sub arr 0sz pivot_idx) pivot_idx; return_array_sub arr _;
    qsort (array_sub arr (SizeT.add pivot_idx 1sz) len)
      (SizeT.sub len (SizeT.add pivot_idx 1sz)); return_array_sub arr _;
    from_mask arr;
  }
}