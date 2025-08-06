module Issue45_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



assume val res : slprop
fn bar_decl ()
requires res
{
admit();
}

fn foo ()
requires res
{
(bar_decl ());
}

fn bar_impl ()
requires res
{
(foo ());
}
