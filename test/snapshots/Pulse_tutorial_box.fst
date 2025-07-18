module Pulse_tutorial_box

#lang-pulse

open Pulse
open Pulse.Lib.C



ghost fn freebie () requires emp ensures pure False { admit(); }
fn test_empty ()
requires emp
returns i:int32
ensures emp
{
0l;
}

fn test_freebie ()
returns i:int32
ensures pure False
{
freebie();
0l;
}

fn new_heap_ref
(v : Int32.t)
returns i:ref int32
ensures i |-> v
ensures freeable i
{
let mut v : Int32.t = v;
let r0 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut r : (ref Int32.t) = r0;
(! r) := (! v);
(! r);
}

fn last_value_of
(r : ref Int32.t)
(#w:_)
requires (r |-> w) ** freeable r
returns i:int32
ensures pure (i == w)
{
let mut r : (ref Int32.t) = r;
let v1 : Int32.t = (! (! r));
let mut v : Int32.t = v1;
(free_ref (! r));
(! v);
}

fn copy_free_box
(r : ref Int32.t)
(#w:_)
requires r |-> w
requires freeable r
returns s:ref int32
ensures s |-> w
ensures freeable s
{
let mut r : (ref Int32.t) = r;
let v2 : Int32.t = (! (! r));
let mut v : Int32.t = v2;
(free_ref (! r));
let s3 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut s : (ref Int32.t) = s3;
(! s) := (! v);
(! s);
}

fn copy_box
(r : ref Int32.t)
(#w:_)
requires r |-> w
returns s:ref int32
ensures (r |-> w) ** (s |-> w) ** freeable s
{
let mut r : (ref Int32.t) = r;
let v4 : Int32.t = (! (! r));
let mut v : Int32.t = v4;
let s5 : (ref Int32.t) = alloc_ref #Int32.t ();
let mut s : (ref Int32.t) = s5;
(! s) := (! v);
(! s);
}
