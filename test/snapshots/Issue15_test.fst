module Issue15_test

#lang-pulse
#set-options "--ext pulse:no_admit_diag"

open Pulse
open Pulse.Lib.C



fn test_empty ()
requires 
emp
ensures 
emp
{
();
}
