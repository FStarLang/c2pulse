module Pulse_tutorial_loops

#lang-pulse

open Pulse
open Pulse.Lib.C



fn count_down
(x : ref Int32.t)
requires exists* v. (x |-> v) ** pure (as_int v >= 0)
ensures x |-> 0l
{
let mut x : (ref Int32.t) = x;
let mut keep_going : Int32.t = 1l;
with vx. assert (x |-> vx);
while((int32_to_bool (bool_to_int32 (Int32.eq (! keep_going) 1l)));
)
invariant b. exists* k v. (keep_going |-> k) ** (x |-> vx) ** (* tedious *) (vx |-> v) ** pure (as_int v >= 0) ** pure (b==(k=1l)) ** pure (k<>1l ==> v==0l)
{
if((int32_to_bool (bool_to_int32 (Int32.eq (! (! x)) 0l))))
{
keep_going := 0l;
}
else
{
(! x) := (Int32.sub (! (! x)) 1l);
};
};
}

fn multiply_by_repeated_addition
(x : Int32.t)
(y : Int32.t)
requires pure (fits ( * ) (as_int x) (as_int y))
requires pure (as_int x >= 0)
returns i:int32
ensures pure (as_int i == as_int x * as_int y)
{
let mut x : Int32.t = x;
let mut y : Int32.t = y;
let mut ctr : Int32.t = 0l;
let mut acc : Int32.t = 0l;
with vx vy. assert (x |-> vx) ** (y |-> vy);
while((int32_to_bool (bool_to_int32 (Int32.lt (! ctr) (! x))));
)
invariant b. exists* c a. (x |-> vx) ** (y |-> vy) ** (* tedious *) (ctr |-> c) ** (acc |-> a) ** pure (as_int c <= as_int vx) ** pure (as_int a == (as_int c * as_int vy)) ** pure (b == (as_int c < as_int vx))
{
ctr := (Int32.add (! ctr) 1l);
acc := (Int32.add (! acc) (! y));
};
(! acc);
}

let rec sum (n:nat) : nat = if n = 0 then 0 else n + sum (n - 1) let rec sum_lemma (n:nat) : Lemma (sum n == n * (n + 1) / 2) = if n = 0 then () else sum_lemma (n - 1) let rec sum_mono (c n:nat) : Lemma (requires c <= n) (ensures sum c <= sum n) [SMTPat (sum c); SMTPat (sum n)] = sum_lemma c; sum_lemma n
fn isum
(n : Int32.t)
requires pure (as_int n >= 0)
requires pure (fits (+) (as_int n) 1)
requires pure (fits ( * ) (as_int n) (as_int n + 1))
returns i:int32
ensures pure (as_int i == (as_int n * (as_int n + 1)) / 2)
{
let mut n : Int32.t = n;
let mut acc : Int32.t = 0l;
let mut ctr : Int32.t = 0l;
sum_lemma(as_int !n);
with vn. assert (n |-> vn);
while((int32_to_bool (bool_to_int32 (Int32.lt (! ctr) (! n))));
)
invariant b. exists* c a. (n |-> vn) ** (* tedious *) (ctr |-> c) ** (acc |-> a) ** pure (as_int c <= as_int vn) ** pure (as_int c >= 0) ** pure (as_int c >= 0 ==> as_int a == sum (as_int c)) ** pure (b == (as_int c < as_int vn))
{
ctr := (Int32.add (! ctr) 1l);
acc := (Int32.add (! acc) (! ctr));
};
(! acc);
}

let rec fib (n:nat) : nat = if n <= 1 then 1 else fib (n - 1) + fib (n - 2) let rec fib_mono (n:nat) (m:nat { m <= n}) : Lemma (ensures fib m <= fib n) = if n = m then () else fib_mono (n - 1) m
fn rec fib_rec
(n : Int32.t)
(cur : ref Int32.t)
(prev : ref Int32.t)
requires pure (as_int n > 0)
requires pure (as_int n > 0 ==> fib (as_int n) <= max_int32)
requires exists* v0 v1. (cur |-> v0) ** (prev |-> v1)
ensures exists* v0 v1. (cur |-> v0) ** (prev |-> v1) ** pure (as_int n > 0 ==> as_int v0 == fib (as_int n)) ** pure (as_int n > 0 ==> as_int v1 == fib (as_int n - 1))
{
let mut n : Int32.t = n;
let mut cur : (ref Int32.t) = cur;
let mut prev : (ref Int32.t) = prev;
if((int32_to_bool (bool_to_int32 (Int32.eq (! n) 1l))))
{
(! cur) := 1l;
(! prev) := 1l;
}
else
{
with vn. assert n |-> vn;
fib_mono(as_int vn) (as_int vn - 1);
(fib_rec (Int32.sub (! n) 1l) (! cur) (! prev));
let mut tmp : Int32.t = (! (! cur));
(! cur) := (Int32.add (! (! cur)) (! (! prev)));
(! prev) := (! tmp);
};
}

fn call_fib_rec
(n : Int32.t)
requires pure (as_int n > 0)
requires pure (as_int n > 0 ==> fib (as_int n) <= max_int32)
returns i:int32
ensures pure (as_int n > 0 ==> as_int i == fib (as_int n))
{
let mut n : Int32.t = n;
let mut cur : Int32.t = 0l;
let mut prev : Int32.t = 0l;
(fib_rec (! n) (cur) (prev));
(! cur);
}
