#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"

INCLUDE (
    open FStar.Ghost
    module SizeT = Pulse.Lib.C.SizeT
    let bool_to_bool b = b

    let my_length = 17
)

#define UDS_LEN 32
#define DICE_SUCCESS 0
#define DICE_ERROR 1
#define DICE_DIGEST_LEN 64 //TODO generalize
#define SHA2_256 0 //maybe use an enum for this?
#define DICE_HASH_ALG SHA2_256 

typedef struct _engine_record_t {
    size_t  l0_image_header_size;
    ISARRAY(l0_image_header_size) uint8_t *l0_image_header;
    ISARRAY(64) uint8_t *l0_image_header_sig;
    size_t  l0_binary_size;
    ISARRAY(l0_binary_size) uint8_t *l0_binary;
    ISARRAY(DICE_DIGEST_LEN) uint8_t *l0_binary_hash;
    ISARRAY(32) uint8_t *l0_image_auth_pubkey;        
} engine_record_t;

INCLUDE (
    module U8 = FStar.UInt8

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
REQUIRES(pubk |-> Frac ppubk pubk_seq) //WOULD BE NICE TO write this as PRESERVES(...)
REQUIRES(hdr |-> Frac phdr hdr_seq)
REQUIRES(sig |-> Frac psig sig_seq)
RETURNS(b:bool)
ENSURES(pubk |-> Frac ppubk pubk_seq)
ENSURES(hdr |-> Frac phdr hdr_seq)
ENSURES(sig |-> Frac psig sig_seq)
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

REQUIRES("a1 |-> Frac 'p1 's1")
REQUIRES("a2 |-> Frac 'p2 's2")
RETURNS(b:bool)
ENSURES("a1 |-> Frac 'p1 's1")
ENSURES("a2 |-> Frac 'p2 's2")
ENSURES("pure (b <==> Seq.equal 's1 's2)")
bool compare (size_t len, ISARRAY(len)uint8_t *a1, ISARRAY(len)uint8_t *a2)
{
    LEMMA(admit());
    return false;
}


REQUIRES(emp)
RETURNS(a:array U8.t)
ENSURES(exists* s. a |-> s)
// ENSURES(freeable a)
ENSURES(pure (length a == SizeT.as_int len))
uint8_t* new_array(size_t len)
{ LEMMA(admit()); return NULL; }

ERASED_ARG(#repr:erased _)
ERASED_ARG(#p:_)
REQUIRES(is_engine_record record p repr)
REQUIRES(exists* s. scratch |-> s)
RETURNS(b:bool)
ENSURES(is_engine_record record p repr)
ENSURES(exists* s. scratch |-> s) //remove scratch parameter and all its uses once array allocation is supported
bool authenticate_l0_image (engine_record_t *record, ISARRAY(DICE_DIGEST_LEN)uint8_t *scratch)
{
    LEMMA(unfold is_engine_record);
    LEMMA(engine_record_t_explode (!record));
    bool valid_header_sig = ed25519_verify(record->l0_image_auth_pubkey, record->l0_image_header, record->l0_image_header_size, record->l0_image_header_sig);
    if (valid_header_sig)
    {
        //allocate a scratch of size DICE_HASH_ALG here and use it
        hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, scratch);
        bool res = compare(DICE_DIGEST_LEN, scratch, record->l0_binary_hash);
        LEMMA(engine_record_t_recover (!record));
        LEMMA(fold is_engine_record);
        return res;
    }
    else
    {
        LEMMA(engine_record_t_recover (!record));
        LEMMA(fold is_engine_record);
        return false;
    }
}


