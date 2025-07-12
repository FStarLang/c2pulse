module Issue43_test

#lang-pulse

open Pulse
open Pulse.Lib.C



fn foo
(p : ref Int32.t)
{
();
}

fn bar ()
{
(foo (null));
}

fn baz ()
{
let p = (null);
(foo p);
}
