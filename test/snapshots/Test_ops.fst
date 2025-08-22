module Test_ops

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn bar_decl
(a : Int32.t)
requires 
pure FStar.Int32.(fits (v a + 1))
returns Int32.t
{
admit();
}

fn test ()
returns Int32.t
{
let mut a : Int32.t = 1l;
pluspluspost_int32 a;
pluspluspre_int32 a;
minusminuspost_int32 a;
minusminuspre_int32 a;
a := Int32.add (! a) 1l;
a := Int32.sub (! a) 1l;
a := Int32.mul (! a) 2l;
a := Int32.div (! a) 1l;
(! a);
}

fn test2
(a : SizeT.t)
requires 
pure FStar.SizeT.(fits (v a + 1))
returns SizeT.t
{
let mut a : SizeT.t = a;
pluspluspost_sizet a;
minusminuspost_sizet a;
pluspluspre_sizet a;
minusminuspre_sizet a;
(! a);
}

fn test3
(a : Int64.t)
requires 
pure FStar.Int64.(fits (v a + 1))
returns Int64.t
{
let mut a : Int64.t = a;
pluspluspost_int64 a;
minusminuspost_int64 a;
pluspluspre_int64 a;
minusminuspre_int64 a;
(! a);
}

fn test4
(a : Int64.t)
requires 
pure FStar.Int64.(fits (v a + 1))
returns Int64.t
{
let mut a : Int64.t = a;
pluspluspost_int64 a;
minusminuspost_int64 a;
pluspluspre_int64 a;
minusminuspre_int64 a;
(! a);
}

fn test5
(a : UInt64.t)
requires 
pure FStar.UInt64.(fits (v a + 1))
returns UInt64.t
{
let mut a : UInt64.t = a;
pluspluspost_uint64 a;
minusminuspost_uint64 a;
pluspluspre_uint64 a;
minusminuspre_uint64 a;
(! a);
}

fn test6
(a : UInt16.t)
requires 
pure FStar.UInt16.(fits (v a + 1))
returns UInt16.t
{
let mut a : UInt16.t = a;
pluspluspost_uint16 a;
minusminuspost_uint16 a;
pluspluspre_uint16 a;
minusminuspre_uint16 a;
(! a);
}

fn test7
(a : UInt8.t)
requires 
pure FStar.UInt8.(fits (v a + 1))
returns UInt8.t
{
let mut a : UInt8.t = a;
pluspluspost_uint8 a;
minusminuspost_uint8 a;
pluspluspre_uint8 a;
minusminuspre_uint8 a;
(! a);
}

fn test8
(a : UInt32.t)
requires 
pure FStar.UInt32.(fits (v a + 1))
returns UInt32.t
{
let mut a : UInt32.t = a;
pluspluspost_uint32 a;
minusminuspost_uint32 a;
pluspluspre_uint32 a;
minusminuspre_uint32 a;
(! a);
}

fn bar_impl
(a : Int32.t)
requires 
pure FStar.Int32.(fits (v a + 1))
returns Int32.t
{
let mut a : Int32.t = a;
pluspluspost_int32 a;
(! a);
}
