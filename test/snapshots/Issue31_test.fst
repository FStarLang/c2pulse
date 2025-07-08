module Issue31_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn check_null
(r : ref Int32.t)
requires emp
returns i:int32
ensures emp
{
if(is_null r)
{
0l;
}
else
{
1l;
};
}
