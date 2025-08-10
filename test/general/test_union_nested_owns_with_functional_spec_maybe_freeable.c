#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"

INCLUDE (
  module U32 = Pulse.Lib.C.UInt32
  open Pulse.Lib.C.UInt32
)

union ab {
  unsigned int *a;
  bool *b;      
};

ERASED_ARG(#a:erased (ref uint32))
ERASED_ARG(#v:erased uint32)
PRESERVES(ab_pred x (Case_ab_a a))
REQUIRES(reveal a |-> v)
REQUIRES(pure (U32.fits ( + ) (UInt32.v v) 1))
ENSURES(exists* (u:uint32). (reveal a |-> u) ** pure (U32.as_int u == U32.as_int v + 1))
void incr_a(union ab *x) {
  LEMMA(ab_is_a (!x));
  LEMMA(ab_explode !x);
  *x->a = *x->a + 1;
  LEMMA(ab_recover !x #(Case_ab_a _));
}

ERASED_ARG(#v:erased _)
REQUIRES(exists* s. ab_pred x s)
PRESERVES(a |-> v)
ENSURES(ab_pred x (Case_ab_a a))
void set_case_a(union ab *x, unsigned int *a) {
  LEMMA(ab_change_a !x);
  LEMMA(ab_explode !x);
  x->a = a;
  LEMMA(ab_recover !x #(Case_ab_a _));
}

typedef struct stru {
        int8_t tag;
        union ab payload;
} stru;

INCLUDE(
let maybe_freeable #a (b:bool) (r:ref a) = 
  if b then freeable r else emp 

let stru_payload (a: ab_spec) (s: either uint32 _Bool) (free:bool) : slprop =
  match a with
  | Case_ab_a a -> 
    exists* v.
      (a |-> v) **
      maybe_freeable free a **
      pure (s == Inl v)
  | Case_ab_b b ->
    exists* v.
      (b |-> v) **
      maybe_freeable free b **
      pure (s == Inr v)

[@@ pulse_unfold]
let tag_relation (s:stru_spec): slprop =
  pure (match s.tag with
        | 0y -> Case_ab_a? s.payload
        | 1y -> Case_ab_b? s.payload
        | _ -> False)

[@@ pulse_unfold]
let stru_ok_aux (u: ref stru) (ib: either uint32 _Bool) (f:bool) (s:_) : slprop =
    stru_pred u s **
    stru_payload s.payload ib f **
    tag_relation s

[@@ pulse_unfold]
let stru_ok (u: ref stru) (ib: either uint32 _Bool) (f:bool) : slprop =
  exists* s. stru_ok_aux u ib f s

ghost
fn elim_stru_payload_a (a:ab_spec { Case_ab_a? a }) (#ib #f:_)
requires stru_payload a ib f
ensures
  exists* v. 
    (Case_ab_a?._0 a |-> v) **
    (maybe_freeable f (Case_ab_a?._0 a)) **
    pure (ib == Inl v)
{
  rewrite each a as (Case_ab_a (Case_ab_a?._0 a));
  unfold stru_payload;
}

ghost
fn elim_stru_payload_b (a:ab_spec { Case_ab_b? a }) (#ib #f:_)
requires stru_payload a ib f
ensures exists* v. 
  (Case_ab_b?._0 a |-> v) **
  (maybe_freeable f (Case_ab_b?._0 a)) **
  pure (ib == Inr v)
{
  rewrite each a as (Case_ab_b (Case_ab_b?._0 a));
  unfold stru_payload;
}

ghost
fn intro_stru_payload_a (a:ab_spec { Case_ab_a? a }) (#f:bool) (#v:_) 
requires (Case_ab_a?._0 a |-> v) ** maybe_freeable f (Case_ab_a?._0 a)
ensures stru_payload (Case_ab_a (Case_ab_a?._0 a)) (Inl v) f
{
  fold stru_payload (Case_ab_a (Case_ab_a?._0 a)) (Inl v) f
}

ghost
fn intro_stru_payload_b (a:ab_spec { Case_ab_b? a }) (#v:_) #f
requires (Case_ab_b?._0 a |-> v) ** maybe_freeable f (Case_ab_b?._0 a)
ensures stru_payload (Case_ab_b (Case_ab_b?._0 a)) (Inr v) f
{
  fold stru_payload (Case_ab_b (Case_ab_b?._0 a)) (Inr v) f
}

ghost
fn intro_stru_ok (u:ref stru) (#s:stru_spec) (#pl:ab_spec) (#ib:_) #f
requires
    stru_pred u s **
    stru_payload pl ib f **
    pure (s.payload == pl) **
    pure (match s.tag with | 0y -> Case_ab_a? s.payload | 1y -> Case_ab_b? s.payload | _ -> False )
ensures stru_ok u ib f
{
  rewrite each pl as s.payload;
}

ghost
fn tag_relation_lemma (y:ref stru) (#ib: either uint32 _Bool) #f (#s:stru_spec) 
preserves stru_ok_aux y ib f s
ensures pure (Inl? ib <==> s.tag==0y)
ensures pure (Inr? ib <==> s.tag==1y)
{
  let v = s.tag;
  if (v = 0y)
  {
    ab_is_a _;
    elim_stru_payload_a _;
    intro_stru_payload_a _;
  }
  else if (v = 1y)
  {
    ab_is_b _;
    elim_stru_payload_b _;
    intro_stru_payload_b _;
  }
  else {
    unreachable()
  }
}
)

ERASED_ARG(#s:erased (either uint32 bool))
ERASED_ARG(#f:erased bool)
REQUIRES(stru_ok foo s f)
ENSURES(exists* (t:either uint32 bool). stru_ok foo t f **
        pure (match s with
              | Inl _ -> t == Inl 1ul
              | Inr _ -> Inr? t && not (Inr?.v t)))
void test_union(struct stru * foo){
  LEMMA(stru_explode (!foo));
  if (foo->tag == 0){
    LEMMA(ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _);
    *foo->payload.a = 1;
    LEMMA(intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _);
          stru_recover (!foo); intro_stru_ok (!foo));
  }
  else if (foo->tag == 1) {
    LEMMA(ab_is_b (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_b _);
    *foo->payload.b = false;
    LEMMA(intro_stru_payload_b _; ab_recover (! (!foo)).payload #(Case_ab_b _);
          stru_recover (!foo); intro_stru_ok (!foo));    
  }
  else {
    LEMMA(unreachable());
  }
}

ERASED_ARG(#s:erased (either uint32 bool) { Inl? s })
ERASED_ARG(#f:erased bool)
REQUIRES(stru_ok foo s f)
ENSURES(stru_ok foo (Inl 1ul) f)
void test_union_a(struct stru * foo){
  LEMMA(tag_relation_lemma _; stru_explode (!foo));
  LEMMA(ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _);
  *foo->payload.a = 1;
  LEMMA(intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _);
        stru_recover (!foo); intro_stru_ok (!foo));
}

ERASED_ARG(#s:erased uint32)
ERASED_ARG(#f:erased bool)
REQUIRES(stru_ok foo (Inl #uint32 s) f)
ENSURES(stru_ok foo (Inl 1ul) f)
void test_union_a_alt(struct stru * foo){
  LEMMA(tag_relation_lemma _; stru_explode (!foo));
  LEMMA(ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _);
  *foo->payload.a = 1;
  LEMMA(intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _);
        stru_recover (!foo); intro_stru_ok (!foo));
}

ERASED_ARG(#s:erased uint32)
ERASED_ARG(#f:erased bool)
REQUIRES(stru_ok foo (Inl #uint32 s) f)
ENSURES(exists* v. stru_ok foo v f ** pure (v == Inl 1ul))
void test_union_a_alt2(stru * foo){
  LEMMA(tag_relation_lemma _; stru_explode (!foo));
  LEMMA(ab_is_a (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_a _);
  *foo->payload.a = 1;
  LEMMA(intro_stru_payload_a _; ab_recover (! (!foo)).payload #(Case_ab_a _);
        stru_recover (!foo); intro_stru_ok (!foo));
}

EXPECT_FAILURE(17)
ERASED_ARG(#s:erased bool)
ERASED_ARG(#f:erased bool)
REQUIRES(stru_ok foo (Inr #_ #bool s) f)
ENSURES(stru_ok foo (Inr (not s)) f)
void test_union_b_alt(stru * foo){
  LEMMA(tag_relation_lemma _; stru_explode (!foo));
  LEMMA(ab_is_b (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_b _);
  *foo->payload.b = !(*foo->payload.b);
  LEMMA(intro_stru_payload_b _; ab_recover (! (!foo)).payload #(Case_ab_b _);
        stru_recover (!foo); intro_stru_ok (!foo));
}

ERASED_ARG(#s:erased bool)
ERASED_ARG(#f:erased bool)
REQUIRES(stru_ok foo (Inr #_ #bool s) f)
ENSURES(exists* v. stru_ok foo v f ** pure (v == Inr (not s)))
void test_union_b_alt2(stru * foo){
  LEMMA(tag_relation_lemma _; stru_explode (!foo));
  LEMMA(ab_is_b (! (! foo)).payload; ab_explode (! (!foo)).payload; elim_stru_payload_b _);
  *foo->payload.b = !(*foo->payload.b);
  LEMMA(intro_stru_payload_b _; ab_recover (! (!foo)).payload #(Case_ab_b _);
        stru_recover (!foo); intro_stru_ok (!foo));
}

INCLUDE (
[@pulse_unfold]
let ab_cases (uv:ab) (s:ab_spec) : slprop = 
begin match s with
 | Case_ab_a v -> uv.a |-> v
 | Case_ab_b v -> uv.b |-> v
end

assume val ab_spec_default : ab_spec

assume val ab_default (ab_var_spec:ab_spec) : ab

instance ab_inhabited : inhabited ab = {
  witness = (ab_default ab_spec_default)
}

ghost
fn ab_pack (ab_var:ref ab) (#ab_spec:ab_spec)
requires ab_var|-> ab_default ab_spec
ensures exists* v. ab_pred ab_var v ** pure (v == ab_spec)
{ admit() }

ghost
fn ab_unpack (ab_var:ref ab) (#v:ab_spec)
requires ab_pred ab_var v
ensures exists* ab. (ab_var |-> ab) ** ab_cases ab v
{ admit() }

ghost fn ab_explode_drop (x : ref ab) (#s : ab_spec)
requires ab_pred x s
ensures exists* (v : ab). (x |-> v)
{
 ab_explode x;
 drop_ (match s with | Case_ab_a _ -> _ | Case_ab_b _ -> _);
}

ghost
fn intro_maybe_freeable a (r:ref a)
requires freeable r
ensures maybe_freeable _true_ r
{
  fold (maybe_freeable _true_ r)
}
)


RETURNS(x:ref stru)
ENSURES(exists* v. stru_ok x v _true_ ** pure (v == Inl 0ul) ** freeable x)
stru* mk_union_a ()
{
  unsigned int* a = (unsigned int*)malloc(sizeof(unsigned int));
  LEMMA(intro_maybe_freeable _ (!a));
  *a = 0ul;
  union ab ab;
  LEMMA(ab_pack ab);
  set_case_a(&ab, a);
  stru *x=(stru*)malloc(sizeof(stru));
  LEMMA(stru_explode (!x));
  x->tag=0;
  LEMMA(ab_unpack ab;
        ab_explode_drop (!(!x)).payload);
  x->payload=ab;
  LEMMA(intro_stru_payload_a (Case_ab_a (!a));
        ab_recover (!(!x)).payload #(Case_ab_a _);
        stru_recover (!x);
        intro_stru_ok (!x));
  return x;
}

