module Union

(* An example of translating unions. *)

#lang-pulse
open Pulse

// union ab {
//         unsigned int a;
//         bool b;
// };

noeq
type ab = {
  a : ref UInt32.t;
  b : ref bool;
}

type ab_spec =
  | Case_a of UInt32.t
  | Case_b of bool

(* Just like a struct, this should the predicate used by the translation
whenever there are fields of type `union ab`. *)
let ab_pred (u : ref ab) (s : ab_spec) : slprop =
  exists* uv. (u |-> uv) **
    begin match s with
    | Case_a v -> uv.a |-> v
    | Case_b v -> uv.b |-> v
    end

ghost
fn ab_explode (x : ref ab) (#s : ab_spec)
  requires ab_pred x s
  ensures exists* (v : ab). (x |-> v) **
    begin match s with
    | Case_a w -> v.a |-> w
    | Case_b w -> v.b |-> w
    end
{
  unfold ab_pred;
}

ghost
fn ab_recover (x : ref ab) (#s : ab_spec)
  requires exists* (v : ab). (x |-> v) **
    begin match s with
    | Case_a w -> v.a |-> w
    | Case_b w -> v.b |-> w
    end
  ensures  ab_pred x s
{
  fold (ab_pred x s);
}

// ERASED_ARG(s:_)
// REQUIRES(pure (Case_a? s))
// REQUIRES(ab_pred x s)
// ENSURES(exists* s'. ab_pred x s')
// void incr_a(union ab *x) {
//   x->a++;
// }

fn incr_a (x : ref ab) (#s : erased ab_spec)
  requires pure (Case_a? s)
  requires ab_pred x s
  ensures  exists* s'. ab_pred x s' 
{
  ab_explode x;
  rewrite each s as Case_a (Case_a?._0 s);
  (!x).a := !(!x).a `UInt32.add_mod` 1ul;
  ab_recover x #(Case_a _);
  ();
}

// struct s {
//         int8_t tag;
//         union ab payload;
// };
noeq
type stru = {
  tag : ref Int8.t;
  payload : ref ab;
}

type stru_spec = {
  tag : Int8.t;
  payload : ab_spec;
}

let stru_pred (u : ref stru) (s : stru_spec) : slprop =
  exists* (uv : stru). (u |-> uv) **
    (uv.tag |-> s.tag) **
    ab_pred uv.payload s.payload

(* User written, just an example. The information relating 0/1 to a/b
   must come from some annotation somewhere, hopefully something simple.
   But that's for later. *)
let stru_ok (u : ref stru) (s : stru_spec) : slprop =
  stru_pred u s **
    pure (
      match s.tag with
      | 0y -> Case_a? s.payload
      | 1y -> Case_b? s.payload
      | _ -> false
    )
