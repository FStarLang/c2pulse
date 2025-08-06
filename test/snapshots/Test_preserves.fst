module Test_preserves

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn foo ()
preserves emp
returns int32
{
0l;
}
