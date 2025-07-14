module Test_bool

#lang-pulse

open Pulse
open Pulse.Lib.C



fn test_bool
(a : Int32.t)
(b : Int32.t)
returns b:_Bool
{
if((Int32.lt a b))
{
true;
}
else
{
false;
};
}
