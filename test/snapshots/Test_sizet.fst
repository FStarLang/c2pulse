module Test_sizet

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test2_decl
(l : SizeT.t)
(k : SizeT.t)
(j : UInt64.t)
returns UInt64.t
{
admit();
}

fn foo_decl
(l : SizeT.t)
requires pure (SizeT.v l < 100)
returns UInt64.t
{
admit();
}

fn foo_impl
(l : SizeT.t)
requires pure (SizeT.v l < 100)
returns UInt64.t
{
let mut l : SizeT.t = l;
let mut y : UInt64.t = (sizet_to_uint64 (! l));
let mut yy : Int32.t = (sizet_to_int32 (! l));
let mut jj : Int64.t = (int32_to_int64 (Int32.add (! yy) 100l));
y := (UInt64.add (! y) (int32_to_uint64 1l));
y := (UInt64.add (! y) (int32_to_uint64 100l));
y := (UInt64.mul (! y) (int32_to_uint64 10l));
(UInt64.add (UInt64.add (! y) (int32_to_uint64 (! yy))) (int64_to_uint64 (! jj)));
}

fn test
(l : SizeT.t)
(k : SizeT.t)
(j : UInt64.t)
requires pure (SizeT.v l < 100)
returns UInt64.t
{
let mut l : SizeT.t = l;
let mut k : SizeT.t = k;
let mut j : UInt64.t = j;
k := (! l);
k := (SizeT.add (! k) (int32_to_sizet 1l));
k := (SizeT.sub (! k) (int32_to_sizet 1l));
j := (foo_decl (! k));
assume (pure (UInt64.v !j < 1000));
k := (uint64_to_sizet (UInt64.add (sizet_to_uint64 (! k)) (! j)));
(sizet_to_uint64 (! k));
}
