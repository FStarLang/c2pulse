module Issue35_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn empty_branch ()
{
let mut x : Int32.t = 1l;
if((int32_to_bool (bool_to_int32 (Int32.eq (! x) 1l))))
{
();
}
else
{
();
};
}
