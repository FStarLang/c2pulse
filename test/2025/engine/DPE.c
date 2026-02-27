#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"
#include "DPE.h"
#include "EngineCore.h"

INCLUDE (
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

ERASED_ARG(#b:erased _)
ERASED_ARG(#p:_)
PRESERVES(a1 |->  Frac p b)
REQUIRES(exists* v. a2 |-> v)
ENSURES(a2 |-> b)
void memcpy_(size_t len, ISARRAY(len)uint8_t *a1, ISARRAY(len)uint8_t *a2)
{
  LEMMA(admit());
}

ERASED_ARG(#uds_bytes:erased (Seq.seq UInt8.t))
ERASED_ARG(#p:_)
PRESERVES(uds |-> Frac p uds_bytes)
RETURNS(c:ref context_t)
ENSURES(exists* s. is_context c s _true_ ** freeable c ** pure (s == PL_Engine uds_bytes))
context_t* init_engine_context(ISARRAY(UDS_LEN) uint8_t *uds)
{
  ISARRAY(UDS_LEN) uint8_t *uds_buf = (uint8_t*)malloc(sizeof(uint8_t)*UDS_LEN);
  LEMMA(intro_maybe (freeable_array (!uds_buf)));
  memcpy_(UDS_LEN, uds, uds_buf);
  context_t *ctx = (context_t*)malloc(sizeof(context_t));
  LEMMA(
    context_t_explode (!ctx);
    u_context_t_change_uds (! (!ctx)).payload;
    u_context_t_explode (! (!ctx)).payload
  );
  ctx->tag = ENGINE_CONTEXT;
  ctx->payload.uds = uds_buf;
  LEMMA(
    intro_owns_payload_a (Case_u_context_t_uds (!uds_buf));
    u_context_t_recover (! (!ctx)).payload #(Case_u_context_t_uds _);
    context_t_recover (!ctx);
    intro_owns_payload (!ctx)
  );
  return ctx;
}

INCLUDE (
  ghost fn elim_maybe_true (p:slprop)
  requires maybe _true_ p
  ensures p
  { unfold maybe; }
)

ERASED_ARG(#uds_bytes:erased (Seq.seq UInt8.t))
ERASED_ARG(#cdi_bytes:erased (Seq.seq UInt8.t))
ERASED_ARG(#p:_)
PRESERVES(cdi |-> Frac p cdi_bytes)
REQUIRES(is_context ctx (PL_Engine uds_bytes) _true_)
ENSURES(is_context ctx (PL_L0 cdi_bytes) _true_)
void init_l0_context(context_t *ctx, ISARRAY(DICE_DIGEST_LEN)uint8_t *cdi)
{
  ISARRAY() uint8_t *cdi_buf = (uint8_t*)malloc(sizeof(uint8_t)*DICE_DIGEST_LEN);
  LEMMA(let _cdi = !cdi_buf; 
        assert pure (length (_cdi) == DICE_DIGEST_LEN));
  LEMMA(intro_maybe (freeable_array (!cdi_buf)));
  memcpy_(DICE_DIGEST_LEN, cdi, cdi_buf);
  LEMMA(tag_relation_lemma _; context_t_explode (!ctx);
        u_context_t_is_uds (!(!ctx)).payload; u_context_t_explode (!(!ctx)).payload;
        elim_owns_payload_a _);
  ISARRAY() uint8_t* uds_buf = ctx->payload.uds;
  LEMMA(elim_maybe_true (freeable_array (!uds_buf)));
  free (uds_buf);
  LEMMA(
    u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_uds _);
    u_context_t_change_cdi (! (!ctx)).payload;
    u_context_t_explode (!(!ctx)).payload
  );
  ctx->tag = 1;
  ctx->payload.cdi = cdi_buf;
  LEMMA(
    intro_owns_payload_b (Case_u_context_t_cdi (!cdi_buf));
    u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_cdi _);
    context_t_recover (!ctx);
    intro_owns_payload (!ctx)
  );
}

REQUIRES(exists* uds. is_context ctx (PL_Engine uds) _true_)
REQUIRES(freeable ctx)
ENSURES(emp)
void destroy_uds_context(context_t *ctx)
{
  LEMMA(tag_relation_lemma _; context_t_explode (!ctx);
        u_context_t_is_uds (!(!ctx)).payload; u_context_t_explode (!(!ctx)).payload;
        elim_owns_payload_a _);
  ISARRAY() uint8_t* uds_buf = ctx->payload.uds;
  LEMMA(elim_maybe_true (freeable_array (!uds_buf)));
  free(uds_buf);
  LEMMA(u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_uds _));
  LEMMA(context_t_recover (!ctx));
  free(ctx);
}

ERASED_ARG(#uds_bytes:erased (Seq.seq UInt8.t))
ERASED_ARG(#cdi_bytes:erased (Seq.seq UInt8.t))
REQUIRES(cdi |-> cdi_bytes)
REQUIRES(freeable_array cdi)
REQUIRES(is_context ctx (PL_Engine uds_bytes) _true_)
ENSURES(is_context ctx (PL_L0 cdi_bytes) _true_)
void mk_l0_context(context_t *ctx, ISARRAY(DICE_DIGEST_LEN)uint8_t *cdi)
{
  LEMMA(let _cdi = !cdi; 
        assert pure (length (_cdi) == DICE_DIGEST_LEN));
  LEMMA(intro_maybe (freeable_array (!cdi)));
  LEMMA(tag_relation_lemma _; context_t_explode (!ctx);
        u_context_t_is_uds (!(!ctx)).payload; u_context_t_explode (!(!ctx)).payload;
        elim_owns_payload_a _);
  ISARRAY() uint8_t* uds_buf = ctx->payload.uds;
  LEMMA(elim_maybe_true (freeable_array (!uds_buf)));
  free (uds_buf);
  LEMMA(
    u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_uds _);
    u_context_t_change_cdi (! (!ctx)).payload;
    u_context_t_explode (!(!ctx)).payload
  );
  ctx->tag = 1;
  ctx->payload.cdi = cdi;
  LEMMA(intro_owns_payload_b (Case_u_context_t_cdi (!cdi)));
  LEMMA(u_context_t_recover (!(!ctx)).payload #(Case_u_context_t_cdi _));
  LEMMA(context_t_recover (!ctx));
  LEMMA(intro_owns_payload (!ctx));
}

ERASED_ARG(#uds:erased _)
ERASED_ARG(#erec:erased _)
ERASED_ARG(#p:perm)
PRESERVES(is_engine_record record p erec)
REQUIRES(freeable ctx)
REQUIRES(is_context ctx (PL_Engine uds) _true_)
RETURNS(ok:_)
ENSURES(maybe ok (freeable ctx ** (exists* cdi. is_context ctx (PL_L0 cdi) _true_)))
bool derive_child_from_context(context_t *ctx, engine_record_t *record)
{
    LEMMA(tag_relation_lemma _; context_t_explode (!ctx));
    u_context_t *pl = &ctx->payload;
    LEMMA(u_context_t_is_uds (!pl); u_context_t_explode (!pl); elim_owns_payload_a _);
    ISARRAY() uint8_t *cdi_buf = (uint8_t*)malloc(sizeof(uint8_t)*DICE_DIGEST_LEN);
    LEMMA(let _cdi = !cdi_buf; 
          assert pure (length (_cdi) == DICE_DIGEST_LEN));
    bool ok = engine_main(cdi_buf, pl->uds, record);
    LEMMA(intro_owns_payload_a (Case_u_context_t_uds !(!(!pl)).uds);
          u_context_t_recover (!pl) #(Case_u_context_t_uds _);
          context_t_recover (!ctx);
          intro_owns_payload (!ctx));
    if (ok)
    {
      mk_l0_context(ctx, cdi_buf);
      LEMMA(
        let c = !ctx; 
        intro_maybe (freeable c ** (exists* cdi. is_context c (PL_L0 cdi) _true_))
      );
      return true;
    }
    else
    {
      destroy_uds_context(ctx);
      free(cdi_buf);
      LEMMA(
        let c = !ctx;
        intro_maybe_false (freeable c ** (exists* cdi. is_context c (PL_L0 cdi) _true_))
      );
      return false;
    }
}