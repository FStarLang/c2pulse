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
(int32_to_bool 1l);
}
else
{
(int32_to_bool 0l);
};
}
