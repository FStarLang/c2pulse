module Issue53_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo
(x : Int32.t)
returns res : ref Int32.t
ensures exists* v. res |-> v
{
let mut x : Int32.t = x;
admit();
(x);
}

[@@expect_failure]
fn bar
(x : Int32.t)
returns Int32.t
{
let mut x : Int32.t = x;
(free_ref (foo (! x)));
42l;
}
