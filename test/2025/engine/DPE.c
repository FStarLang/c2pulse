#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../c2pulse.h"
#include "DPE.h"
#include "EngineCore.h"

_include_pulse(
module U8 = FStar.UInt8

noeq
type context_payload_spec =
| PL_Engine : uds:Seq.seq U8.t -> context_payload_spec
| PL_L0     : cdi:Seq.seq U8.t -> context_payload_spec
| PL_L1 //TBD

let owns_payload (c: u_context_t_spec) (s: context_payload_spec) (f:bool) : slprop =
  match c with
  | Case_u_context_t_uds uds ->
    exists* v. 
      (uds |-> v) **
      maybe f (freeable_array uds) **
      pure (s == PL_Engine v)
  | Case_u_context_t_cdi cdi ->
    exists* v.
      (cdi |-> v) **
      maybe f (freeable_array cdi) **
      pure (s == PL_L0 v)
  | Case_u_context_t_l1_context l1 ->
    pure (s == PL_L1)

[@@ pulse_unfold]
let tag_relation (s:context_t_spec): slprop =
  pure (match s.tag with
        | 0uy -> Case_u_context_t_uds? s.payload
        | 1uy -> Case_u_context_t_cdi? s.payload
        | 2uy -> Case_u_context_t_l1_context? s.payload
        | _ -> False)

[@@ pulse_unfold]
let owns_context_t_aux (u: ref context_t) (pl: context_payload_spec) (f:bool) (s:context_t_spec) : slprop =
    context_t_pred u s **
    owns_payload s.payload pl f **
    tag_relation s

[@@ pulse_unfold]
let is_context (u: ref context_t) (pl: context_payload_spec) (f:bool) : slprop =
  exists* s. owns_context_t_aux u pl f s

ghost fn intro_owns_payload_a (a:u_context_t_spec { Case_u_context_t_uds? a }) (#f:_Bool) (#v:_) requires (Case_u_context_t_uds?._0 a |-> v) ** maybe f (freeable_array (Case_u_context_t_uds?._0 a)) ensures owns_payload (Case_u_context_t_uds (Case_u_context_t_uds?._0 a)) (PL_Engine v) f { fold owns_payload (Case_u_context_t_uds (Case_u_context_t_uds?._0 a)) (PL_Engine v) f }

ghost
fn elim_owns_payload_a (a:u_context_t_spec { Case_u_context_t_uds? a }) (#ib #f:_)
requires owns_payload a ib f
ensures
  exists* v. 
    (Case_u_context_t_uds?._0 a |-> v) **
    (maybe f (freeable_array (Case_u_context_t_uds?._0 a))) **
    pure (ib == PL_Engine v)
{
  rewrite each a as (Case_u_context_t_uds (Case_u_context_t_uds?._0 a));
  unfold owns_payload;
}

ghost fn intro_owns_payload_b (a:u_context_t_spec { Case_u_context_t_cdi? a }) (#f:_Bool) (#v:_) 
requires (Case_u_context_t_cdi?._0 a |-> v) ** maybe f (freeable_array (Case_u_context_t_cdi?._0 a))
ensures owns_payload (Case_u_context_t_cdi (Case_u_context_t_cdi?._0 a)) (PL_L0 v) f 
{ fold owns_payload (Case_u_context_t_cdi (Case_u_context_t_cdi?._0 a)) (PL_L0 v) f }

ghost
fn elim_owns_payload_b (a:u_context_t_spec { Case_u_context_t_cdi? a }) (#ib #f:_)
requires owns_payload a ib f
ensures
  exists* v. 
    (Case_u_context_t_cdi?._0 a |-> v) **
    (maybe f (freeable_array (Case_u_context_t_cdi?._0 a))) **
    pure (ib == PL_L0 v)
{
  rewrite each a as (Case_u_context_t_cdi (Case_u_context_t_cdi?._0 a));
  unfold owns_payload;
}

ghost fn intro_owns_payload (u:ref context_t) (#s:context_t_spec) (#pl:u_context_t_spec) (#ib:context_payload_spec) #f 
requires context_t_pred u s ** owns_payload pl ib f ** pure (s.payload == pl) ** tag_relation s
ensures is_context u ib f { rewrite each pl as s.payload; }

ghost
fn tag_relation_lemma (u:ref context_t) (#ib: context_payload_spec) #f (#s:context_t_spec) 
preserves owns_context_t_aux u ib f s
ensures pure (PL_Engine? ib <==> s.tag==0uy)
ensures pure (PL_L0? ib <==> s.tag==1uy)
ensures pure (PL_L1? ib <==> s.tag==2uy)
{
  let v = s.tag;
  if (v = 0uy)
  {
    u_context_t_is_uds _;
    elim_owns_payload_a _;
    intro_owns_payload_a _;
  }
  else if (v = 1uy)
  {
    u_context_t_is_cdi _;
    elim_owns_payload_b _;
    intro_owns_payload_b _;
  }
  else {
    admit()
    // unreachable()
  }
}
)

_preserves((_slprop) _inline_pulse(a1 |->  Frac p b))
_requires((_slprop) _inline_pulse(exists* v. a2 |-> v))
_ensures((_slprop) _inline_pulse(a2 |-> b))
void memcpy_(size_t len, uint8_t *a1, uint8_t *a2)
{
  _assert((_slprop) _inline_pulse(admit()));
}

_preserves((_slprop) _inline_pulse(uds |-> Frac p uds_bytes))
_ensures((_slprop) _inline_pulse(exists* s. is_context c s _true_ ** freeable c ** pure (s == PL_Engine uds_bytes)))
context_t* init_engine_context(uint8_t *uds)
{
  uint8_t *uds_buf = (uint8_t*)malloc(sizeof(uint8_t)*UDS_LEN);
  _assert((_slprop) _inline_pulse(intro_maybe (freeable_array (!uds_buf))));
  memcpy_(UDS_LEN, uds, uds_buf);
  context_t *ctx = (context_t*)malloc(sizeof(context_t));
  _assert((_slprop) _inline_pulse(context_t_explode (!ctx);
    u_context_t_change_uds (! (!ctx)).payload;
    u_context_t_explode (! (!ctx)).payload));
  ctx->tag = ENGINE_CONTEXT;
  ctx->payload.uds = uds_buf;
  _assert((_slprop) _inline_pulse(intro_owns_payload_a (Case_u_context_t_uds (!uds_buf));
    u_context_t_recover (! (!ctx)).payload #(Case_u_context_t_uds _);
    context_t_recover (!ctx);
    intro_owns_payload (!ctx)));
  return ctx;
}

_include_pulse(
  ghost fn elim_maybe_true (p:slprop)
  requires maybe _true_ p
  ensures p
  { unfold maybe; }
)

_preserves((_slprop) _inline_pulse(cdi |-> Frac p cdi_bytes))
_requires((_slprop) _inline_pulse(is_context ctx (PL_Engine uds_bytes) _true_))
_ensures((_slprop) _inline_pulse(is_context ctx (PL_L0 cdi_bytes) _true_))
void init_l0_context(context_t *ctx, uint8_t *cdi)
{
  uint8_t *cdi_buf = (uint8_t*)malloc(sizeof(uint8_t)*DICE_DIGEST_LEN);
  _assert((_slprop) _inline_pulse(let _cdi = !cdi_buf; 
        assert pure (length (_cdi) == DICE_DIGEST_LEN)));
  _assert((_slprop) _inline_pulse(intro_maybe (freeable_array (!cdi_buf))));
  memcpy_(DICE_DIGEST_LEN, cdi, cdi_buf);
  _assert((_slprop) _inline_pulse(tag_relation_lemma _; context_t_explode (!ctx);
        u_context_t_is_uds (!(!ctx)).payload; u_context_t_explode (!(!ctx)).payload;
        elim_owns_payload_a _));
  uint8_t* uds_buf = ctx->payload.uds;
  _assert((_slprop) _inline_pulse(elim_maybe_true (freeable_array (!uds_buf))));
  free (uds_buf);
  _assert((_slprop) _inline_pulse(u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_uds _);
    u_context_t_change_cdi (! (!ctx)).payload;
    u_context_t_explode (!(!ctx)).payload));
  ctx->tag = 1;
  ctx->payload.cdi = cdi_buf;
  _assert((_slprop) _inline_pulse(intro_owns_payload_b (Case_u_context_t_cdi (!cdi_buf));
    u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_cdi _);
    context_t_recover (!ctx);
    intro_owns_payload (!ctx)));
}

_requires((_slprop) _inline_pulse(exists* uds. is_context ctx (PL_Engine uds) _true_))
_requires((_slprop) _inline_pulse(freeable ctx))
void destroy_uds_context(context_t *ctx)
{
  _assert((_slprop) _inline_pulse(tag_relation_lemma _; context_t_explode (!ctx);
        u_context_t_is_uds (!(!ctx)).payload; u_context_t_explode (!(!ctx)).payload;
        elim_owns_payload_a _));
  uint8_t* uds_buf = ctx->payload.uds;
  _assert((_slprop) _inline_pulse(elim_maybe_true (freeable_array (!uds_buf))));
  free(uds_buf);
  _assert((_slprop) _inline_pulse(u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_uds _)));
  _assert((_slprop) _inline_pulse(context_t_recover (!ctx)));
  free(ctx);
}

_requires((_slprop) _inline_pulse(cdi |-> cdi_bytes))
_requires((_slprop) _inline_pulse(freeable_array cdi))
_requires((_slprop) _inline_pulse(is_context ctx (PL_Engine uds_bytes) _true_))
_ensures((_slprop) _inline_pulse(is_context ctx (PL_L0 cdi_bytes) _true_))
void mk_l0_context(context_t *ctx, uint8_t *cdi)
{
  _assert((_slprop) _inline_pulse(let _cdi = !cdi; 
        assert pure (length (_cdi) == DICE_DIGEST_LEN)));
  _assert((_slprop) _inline_pulse(intro_maybe (freeable_array (!cdi))));
  _assert((_slprop) _inline_pulse(tag_relation_lemma _; context_t_explode (!ctx);
        u_context_t_is_uds (!(!ctx)).payload; u_context_t_explode (!(!ctx)).payload;
        elim_owns_payload_a _));
  uint8_t* uds_buf = ctx->payload.uds;
  _assert((_slprop) _inline_pulse(elim_maybe_true (freeable_array (!uds_buf))));
  free (uds_buf);
  _assert((_slprop) _inline_pulse(u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_uds _);
    u_context_t_change_cdi (! (!ctx)).payload;
    u_context_t_explode (!(!ctx)).payload));
  ctx->tag = 1;
  ctx->payload.cdi = cdi;
  _assert((_slprop) _inline_pulse(intro_owns_payload_b (Case_u_context_t_cdi (!cdi))));
  _assert((_slprop) _inline_pulse(u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_cdi _)));
  _assert((_slprop) _inline_pulse(context_t_recover (!ctx)));
  _assert((_slprop) _inline_pulse(intro_owns_payload (!ctx)));
}

_preserves((_slprop) _inline_pulse(is_engine_record record p erec))
_requires((_slprop) _inline_pulse(freeable ctx))
_requires((_slprop) _inline_pulse(is_context ctx (PL_Engine uds) _true_))
_ensures((_slprop) _inline_pulse(maybe ok (freeable ctx ** (exists* cdi. is_context ctx (PL_L0 cdi) _true_))))
bool derive_child_from_context(context_t *ctx, engine_record_t *record)
{
    _assert((_slprop) _inline_pulse(tag_relation_lemma _; context_t_explode (!ctx)));
    u_context_t *pl = &ctx->payload;
    _assert((_slprop) _inline_pulse(u_context_t_is_uds (!pl); u_context_t_explode (!pl); elim_owns_payload_a _));
    uint8_t *cdi_buf = (uint8_t*)malloc(sizeof(uint8_t)*DICE_DIGEST_LEN);
    _assert((_slprop) _inline_pulse(let _cdi = !cdi_buf; 
          assert pure (length (_cdi) == DICE_DIGEST_LEN)));
    bool ok = engine_main(cdi_buf, pl->uds, record);
    _assert((_slprop) _inline_pulse(intro_owns_payload_a (Case_u_context_t_uds !(!(!pl)).uds);
          u_context_t_recover (!pl) #(Case_u_context_t_uds _);
          context_t_recover (!ctx);
          intro_owns_payload (!ctx)));
    if (ok)
    {
      mk_l0_context(ctx, cdi_buf);
      _assert((_slprop) _inline_pulse(let c = !ctx; 
        intro_maybe (freeable c ** (exists* cdi. is_context c (PL_L0 cdi) _true_))));
      return true;
    }
    else
    {
      destroy_uds_context(ctx);
      free(cdi_buf);
      _assert((_slprop) _inline_pulse(let c = !ctx;
        intro_maybe_false (freeable c ** (exists* cdi. is_context c (PL_L0 cdi) _true_))));
      return false;
    }
}