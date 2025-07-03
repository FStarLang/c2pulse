module Issue15_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test_empty ()
requires emp
ensures emp
{
();
}
