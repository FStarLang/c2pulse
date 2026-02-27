#ifndef _ENGINE_CORE_H_
#define _ENGINE_CORE_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"
#include "DPE.h"

INCLUDE (
    open FStar.Ghost
    module SizeT = Pulse.Lib.C.SizeT
    let bool_to_bool b = b

    let my_length = 17
)

INCLUDE (
    module U8 = FStar.UInt8
    module US = FStar.SizeT


    noeq
    type engine_record_repr = {
        l0_image_header      : Seq.seq U8.t;
        l0_image_header_sig  : Seq.seq U8.t;
        l0_binary            : Seq.seq U8.t;
        l0_binary_hash       : Seq.seq U8.t;
        l0_image_auth_pubkey : Seq.seq U8.t;
    }

    [@@pulse_unfold]
    let engine_record_perm (record:engine_record_t_spec) (p:perm) (repr:engine_record_repr)
    : slprop
    =  (record.l0_image_header |-> Frac p repr.l0_image_header) **
       (record.l0_image_header_sig |-> Frac p repr.l0_image_header_sig) **
       (record.l0_binary |-> Frac p repr.l0_binary) **
       (record.l0_binary_hash |-> Frac p repr.l0_binary_hash) **
       (record.l0_image_auth_pubkey |-> Frac p repr.l0_image_auth_pubkey)

    [@@pulse_unfold]
    let is_engine_record (r:ref engine_record_t) (p:perm) (repr:engine_record_repr) =
        exists* s. engine_record_t_pred r s ** engine_record_perm s p repr

)

ERASED_ARG(#repr:erased _)
ERASED_ARG(#uds_perm:_)
ERASED_ARG(#p:_)
ERASED_ARG(#uds_bytes:erased _)
PRESERVES(is_engine_record record p repr)
PRESERVES(uds |-> Frac uds_perm uds_bytes)
REQUIRES(exists* c0. cdi |-> c0)
ENSURES(exists* c1. cdi |-> c1)
bool engine_main(ISARRAY(DICE_DIGEST_LEN) uint8_t *cdi, ISARRAY(UDS_LEN) uint8_t *uds, engine_record_t *record);

#endif