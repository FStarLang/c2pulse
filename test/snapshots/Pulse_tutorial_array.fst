module Pulse_tutorial_array

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C




module SZ = Pulse.Lib.C.SizeT
module U64 = FStar.UInt64
fn read_i
(arr : array Int32.t)
(i : SizeT.t)
(#s:erased (Seq.seq int32))
(#p:perm { SizeT.v i < Seq.length s })
requires 
arr |-> Frac p s
returns v:int32
ensures 
arr |-> Frac p s
ensures 
pure (v == Seq.index s (SizeT.v i) )
{
let mut arr : (array Int32.t) = arr;
let mut i : SizeT.t = i;
(op_Array_Access (! arr) (! i));
}

fn write_i
(arr : array Int32.t)
(i : SizeT.t)
(v : Int32.t)
(#s:erased (Seq.seq int32) { SizeT.v i < Seq.length s })
requires 
arr |-> s
ensures 
exists* s1. (arr |-> s1) ** pure (s1 == Seq.upd s (SizeT.v i) v)
{
let mut arr : (array Int32.t) = arr;
let mut i : SizeT.t = i;
let mut v : Int32.t = v;
(! arr).((! i)) <- (! v);
}

fn compare_elements
(a1 : array Int32.t)
(a2 : array Int32.t)
(l : SizeT.t)
(i : SizeT.t)
(#s1 #s2 : erased (Seq.seq int32))
(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = SizeT.v l })
preserves 
a1 |-> Frac p s1
preserves 
a2 |-> Frac p s2
returns res:int32
ensures 
pure (res==1l <==> (SizeT.v i < SizeT.v l && Seq.index s1 (SizeT.v i) = Seq.index s2 (SizeT.v i)))
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut l : SizeT.t = l;
let mut i : SizeT.t = i;
if((int32_to_bool (bool_to_int32 (SizeT.lt (! i) (! l)))))
{
if((int32_to_bool (bool_to_int32 (Int32.eq (op_Array_Access (! a1) (! i)) (op_Array_Access (! a2) (! i))))))
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
(#p:perm { Seq.length s1 = Seq.length s2 && Seq.length s2 = SizeT.v l })
preserves 
a1 |-> Frac p s1
preserves 
a2 |-> Frac p s2
returns res:_Bool
ensures 
pure (res <==> Seq.equal s1 s2)
{
let mut a1 : (array Int32.t) = a1;
let mut a2 : (array Int32.t) = a2;
let mut l : SizeT.t = l;
let mut i : SizeT.t = (int32_to_sizet 0l);
with va1. assert (a1 |-> va1);
with va2. assert (a2 |-> va2);
while((int32_to_bool (bool_to_int32 (Int32.eq (compare_elements (! a1) (! a2) (! l) (! i)) 1l)));
)


invariant b.
exists* vi vl.
(a1 |-> va1) ** (a2 |-> va2) ** (* tedious *)
(i |-> vi) ** (va1 |-> Frac p s1) ** (va2 |-> Frac p s2) **
(l |-> vl) **
pure (
Seq.length s1 = SizeT.v vl
/\ Seq.length s2 = SizeT.v vl
/\ SizeT.v vi <= SizeT.v vl
/\ (b == (SizeT.v vi < SizeT.v vl && Seq.index s1 (SizeT.v vi) = Seq.index s2 (SizeT.v vi)))
/\ (forall (i:nat). i < SizeT.v vi ==> Seq.index s1 i == Seq.index s2 i))
{
i := (SizeT.add (! i) (int32_to_sizet 1l));
};
(int32_to_bool ((bool_to_int32 (SizeT.eq (! i) (! l)))));
}
