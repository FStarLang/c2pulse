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

REQUIRES(exists* s. engine_record_t_pred r s)
ENSURES(exists* s. engine_record_t_pred r s)
void test (engine_record_t *r)
{
    LEMMA(engine_record_t_explode (!r));
    LEMMA(engine_record_t_recover (!r));
}



ERASED_ARG(#ppubk #phdr #psig:_)
ERASED_ARG(#pubk_seq #hdr_seq #sig_seq:erased (Seq.seq U8.t))
PRESERVES(pubk |-> Frac ppubk pubk_seq)
PRESERVES(hdr |-> Frac phdr hdr_seq)
PRESERVES(sig |-> Frac psig sig_seq)
bool ed25519_verify(ISARRAY() uint8_t *pubk, ISARRAY() uint8_t *hdr, size_t hdr_len, ISARRAY() uint8_t *sig)
{ return false; }



REQUIRES("dst |-> 'dst_seq")
REQUIRES("src |-> Frac 'p 'src_seq")
ENSURES("src |-> Frac 'p 'src_seq")
ENSURES("exists* dst_seq. dst |-> dst_seq")
void hacl_hash (uint8_t alg, size_t src_len, ISARRAY(src_len)uint8_t *src, ISARRAY(DICE_DIGEST_LEN)uint8_t *dst)
{
    return;
}


REQUIRES("dst |-> 'dst_seq")
REQUIRES("key |-> Frac 'p 'key_seq")
REQUIRES("msg |-> Frac 'p 'msg_seq")
ENSURES("key |-> Frac 'p 'key_seq")
ENSURES("msg |-> Frac 'p 'msg_seq")
ENSURES("exists* dst_seq. dst |-> dst_seq")
void hacl_hmac(uint8_t alg, ISARRAY(DICE_DIGEST_LEN)uint8_t *dst, ISARRAY(key_len)uint8_t *key, size_t key_len, ISARRAY(msg_len)uint8_t *msg, size_t msg_len)
{
    return;
}

RETURNS(SizeT.t)
size_t hashf(uint16_t key){
    LEMMA(admit());
}

REQUIRES("a1 |-> Frac 'p1 's1")
REQUIRES("a2 |-> Frac 'p2 's2")
RETURNS(b:bool)
ENSURES("a1 |-> Frac 'p1 's1")
ENSURES("a2 |-> Frac 'p2 's2")
ENSURES("pure (b <==> Seq.equal 's1 's2)")
bool compare(size_t len, ISARRAY(len)uint8_t *a1, ISARRAY(len)uint8_t *a2)
{
    LEMMA(admit());
    return false;
}


REQUIRES(emp)
RETURNS(a:array U8.t)
ENSURES(exists* s. a |-> s)
ENSURES(pure (length a == SizeT.v len))
uint8_t* new_array(size_t len)
{ LEMMA(admit()); return NULL; }

ERASED_ARG(#repr:erased _)
ERASED_ARG(#p:_)
PRESERVES(is_engine_record record p repr)
bool authenticate_l0_image(engine_record_t *record)
{
    bool valid_header_sig = ed25519_verify(record->l0_image_auth_pubkey, record->l0_image_header, record->l0_image_header_size, record->l0_image_header_sig);
    if (valid_header_sig)
    {   
        uint8_t scratch[DICE_DIGEST_LEN];
        //allocate a scratch of size DICE_HASH_ALG here and use it
        hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, scratch);
        bool res = compare(DICE_DIGEST_LEN, scratch, record->l0_binary_hash);
        return res;
    }
    else
    {
        return false;
    }
}

ERASED_ARG(#uds_perm:_)
ERASED_ARG(#p:_)
ERASED_ARG(#repr:erased _)
PRESERVES(is_engine_record record p repr)
ERASED_ARG(#uds_bytes:erased _)
PRESERVES(uds |-> Frac uds_perm uds_bytes)
REQUIRES(exists* c0. cdi |-> c0)
ENSURES(exists* c1. cdi |-> c1)
void compute_cdi(ISARRAY(DICE_DIGEST_LEN)uint8_t *cdi, ISARRAY(UDS_LEN)uint8_t *uds, engine_record_t *record){
    uint8_t uds_digest[DICE_DIGEST_LEN];
    uint8_t l0_digest[DICE_DIGEST_LEN];
    hacl_hash(DICE_HASH_ALG, UDS_LEN, uds, uds_digest);
    hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, l0_digest);
    hacl_hmac(DICE_HASH_ALG, cdi, uds_digest, DICE_DIGEST_LEN, l0_digest, DICE_DIGEST_LEN);
}

ERASED_ARG(#repr:erased _)
ERASED_ARG(#uds_perm:_)
ERASED_ARG(#p:_)
ERASED_ARG(#uds_bytes:erased _)
PRESERVES(is_engine_record record p repr)
PRESERVES(uds |-> Frac uds_perm uds_bytes)
REQUIRES(exists* c0. cdi |-> c0)
ENSURES(exists* c1. cdi |-> c1)
bool engine_main(ISARRAY(DICE_DIGEST_LEN) uint8_t *cdi, ISARRAY(UDS_LEN) uint8_t *uds, engine_record_t *record){

    bool b = authenticate_l0_image(record);
    if (b){
        compute_cdi(cdi, uds, record);
        return true;
    }
    else{
        return false;
    }
}