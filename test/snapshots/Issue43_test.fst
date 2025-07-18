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
(foo null);
}

fn baz ()
{
let p0 : (ref Int32.t) = null;
let mut p : (ref Int32.t) = p0;
(foo (! p));
}
