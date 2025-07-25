module Pulse_tutorial_array

#lang-pulse

open Pulse
open Pulse.Lib.C



module SZ = Pulse.Lib.C.SizeT module U64 = FStar.UInt64
fn read_i
(arr : array Int32.t)
(i : UInt64.t)
(#s:erased (Seq.seq int32))
(#p:perm { U64.v i < Seq.length s })
requires arr |-> Frac p s
returns v:int32
ensures arr |-> Frac p s
ensures pure (v == Seq.index s (U64.v i) )
{
let mut arr : (array Int32.t) = arr;
let mut i : UInt64.t = i;
(op_Array_Access (! arr) (uint64_to_sizet (! i)));
}

fn write_i
(arr : array Int32.t)
(i : UInt64.t)
(v : Int32.t)
(#s:erased (Seq.seq int32) { U64.v i < Seq.length s })
requires arr |-> s
ensures exists* s1. (arr |-> s1) ** pure (s1 == Seq.upd s (U64.v i) v)
{
let mut arr : (array Int32.t) = arr;
let mut i : UInt64.t = i;
let mut v : Int32.t = v;
(! arr).(uint64_to_sizet (! i)) <- (! v);
}

fn compare_elements
(a1 : array Int32.t)
(a2 : array Int32.t)
(l : UInt64.t)
(i : UInt64.t)
(#s1 #s2 : erased (Seq.seq int32))
(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = U64.v l })
requires a1 |-> Frac p s1
requires a2 |-> Frac p s2
returns res:int32
ensures a1 |-> Frac p s1
ensures a2 |-> Frac p s2
ensures pure (res==1l <==> (U64.v i < U64.v l && Seq.index s1 (U64.v i) = Seq.index s2 (U64.v i)))
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut l : UInt64.t = l;
let mut i : UInt64.t = i;
if((int32_to_bool (bool_to_int32 (UInt64.lt (! i) (! l)))))
{
if((int32_to_bool (bool_to_int32 (Int32.eq (op_Array_Access (! a1) (uint64_to_sizet (! i))) (op_Array_Access (! a2) (uint64_to_sizet (! i)))))))
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
(l : UInt64.t)
(#s1 #s2 : erased (Seq.seq int32))
(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = U64.v l })
requires a1 |-> Frac p s1
requires a2 |-> Frac p s2
returns res:_Bool
ensures a1 |-> Frac p s1
ensures a2 |-> Frac p s2
ensures pure (res <==> Seq.equal s1 s2)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut l : UInt64.t = l;
let mut i : UInt64.t = (int32_to_uint64 0l);
with va1. assert (a1 |-> va1);
with va2. assert (a2 |-> va2);
while((int32_to_bool (bool_to_int32 (Int32.eq (compare_elements (! a1) (! a2) (! l) (! i)) 1l)));
)
invariant b. exists* vi vl. (a1 |-> va1) ** (a2 |-> va2) ** (* tedious *) (i |-> vi) ** (va1 |-> Frac p s1) ** (va2 |-> Frac p s2) **(l |-> vl) **pure (Seq.length s1 = U64.v vl /\Seq.length s2 = U64.v vl /\U64.v vi <= U64.v vl /\(b == (U64.v vi < U64.v vl && Seq.index s1 (U64.v vi) = Seq.index s2 (U64.v vi))) /\(forall (i:nat). i < U64.v vi ==> Seq.index s1 i == Seq.index s2 i))
{
i := (UInt64.add (! i) (int32_to_uint64 1l));
};
(int32_to_bool ((bool_to_int32 (UInt64.eq (! i) (! l)))));
}
