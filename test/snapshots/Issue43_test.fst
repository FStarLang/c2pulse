module Issue43_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(p : ref Int32.t)
{
let mut p : (ref Int32.t) = p;
();
}

fn bar ()
{
(foo (null #_));
}

fn baz ()
{
let mut p : (ref Int32.t) = (null #_);
(foo (! p));
}
