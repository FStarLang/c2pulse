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
