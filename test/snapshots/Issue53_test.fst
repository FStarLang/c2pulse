module Issue53_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo
(x : Int32.t)
returns res : ref Int32.t
ensures 
exists* v. (res |-> v) ** freeable res
{
let mut x : Int32.t = x;
alloc_ref #Int32.t ();
}

fn bar
(x : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
(free_ref (foo (! x)));
42l;
}
