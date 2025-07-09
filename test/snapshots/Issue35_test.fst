module Issue35_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn empty_branch ()
{
let x = 1l;
if((Int32.eq x 1l))
{
()
}
else
{
()
};
}
