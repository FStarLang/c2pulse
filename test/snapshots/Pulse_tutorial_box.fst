module Pulse_tutorial_box

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C




ghost
fn freebie ()
requires emp
ensures pure False
{
admit();
}
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
let mut r : (ref Int32.t) = alloc_ref #Int32.t ();
(! r) := (! v);
(! r);
}

fn last_value_of
(r : ( ref Int32.t) )
(#w:_)
requires (r |-> w) ** freeable r
returns i:int32
ensures pure (i == w)
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = (! (! r));
(free_ref (! r));
(! v);
}

fn copy_free_box
(r : ( ref Int32.t) )
(#w:_)
requires r |-> w
requires freeable r
returns s:ref int32
ensures s |-> w
ensures freeable s
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = (! (! r));
(free_ref (! r));
let mut s : (ref Int32.t) = alloc_ref #Int32.t ();
(! s) := (! v);
(! s);
}

fn copy_box
(r : ( ref Int32.t) )
(#w:_)
requires r |-> w
returns s:ref int32
ensures (r |-> w) ** (s |-> w) ** freeable s
{
let mut r : (ref Int32.t) = r;
let mut v : Int32.t = (! (! r));
let mut s : (ref Int32.t) = alloc_ref #Int32.t ();
(! s) := (! v);
(! s);
}
