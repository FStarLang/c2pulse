module Test_preserves

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo ()
preserves emp
returns int32
{
0l;
}
