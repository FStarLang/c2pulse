module Issue44_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn bar_decl ()
{
admit();
}

fn bar_impl ()
{
let mut x : Int32.t = 1l;
();
}
