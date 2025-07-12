module Pulse_tutorial_array

#lang-pulse

open Pulse
open Pulse.Lib.C



module SZ = Pulse.Lib.C.SizeT
fn read_i
(arr : array Int32.t)
(i : SizeT.t)
(#s:erased (Seq.seq int32))
(#p:perm { SZ.as_int i < Seq.length s })
requires arr |-> Frac p s
returns v:int32
ensures arr |-> Frac p s
ensures pure (v == Seq.index s (SZ.as_int i) )
{
(op_Array_Access arr i);
}

fn write_i
(arr : array Int32.t)
(i : SizeT.t)
(v : Int32.t)
(#s:erased (Seq.seq int32) { SZ.as_int i < Seq.length s })
requires arr |-> s
ensures exists* s1. (arr |-> s1) ** pure (s1 == Seq.upd s (SZ.as_int i) v)
{
arr.(i) <- v;
}

fn compare_elements
(a1 : array Int32.t)
(a2 : array Int32.t)
(l : SizeT.t)
(i : SizeT.t)
(#s1 #s2 : erased (Seq.seq int32))
(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = SZ.as_int l })
requires a1 |-> Frac p s1
requires a2 |-> Frac p s2
returns res:int32
ensures a1 |-> Frac p s1
ensures a2 |-> Frac p s2
ensures pure (res==1l <==> (SZ.as_int i < SZ.as_int l && Seq.index s1 (SZ.as_int i) = Seq.index s2 (SZ.as_int i)))
{
if((SizeT.lt i l))
{
if((Int32.eq (op_Array_Access a1 i) (op_Array_Access a2 i)))
{
1l;
}
else
{
0l;
};
}
else
{
0l;
};
}

module SizeT = Pulse.Lib.C.SizeT
fn compare
(a1 : array Int32.t)
(a2 : array Int32.t)
(l : SizeT.t)
(#s1 #s2 : erased (Seq.seq int32))
(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = SZ.as_int l })
requires a1 |-> Frac p s1
requires a2 |-> Frac p s2
returns res:_Bool
ensures a1 |-> Frac p s1
ensures a2 |-> Frac p s2
ensures pure (res <==> Seq.equal s1 s2)
{
let mut i = 0sz;
while((Int32.eq (compare_elements a1 a2 l (! i)) 1l);
)
invariant b.        exists* vi.            (i |-> vi) ** (a1 |-> Frac p s1) ** (a2 |-> Frac p s2) **            pure (                SZ.as_int vi <= SZ.as_int l /\                 (b == (SZ.as_int vi < SZ.as_int l && Seq.index s1 (SZ.as_int vi) = Seq.index s2 (SZ.as_int vi))) /\                 (forall (i:nat). i < SZ.as_int vi ==> Seq.index s1 i == Seq.index s2 i)                        )
{
i := (SizeT.add (! i) 1sz);
};((SizeT.eq (! i) l));
}
