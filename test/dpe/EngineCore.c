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

    let is_engine_record (r:ref engine_record_t) (p:perm) (repr:engine_record_repr) =
        exists* s. engine_record_t_pred r s ** engine_record_perm s p repr

    // let spec_hash a s = EverCrypt.Hash.Incremental.spec_hash a s


    // let cdi_functional_correctness (c0:Seq.seq U8.t) (uds_bytes:Seq.seq U8.t) (repr:engine_record_repr) 
    // : prop 
    // = Seq.length uds_bytes == US.v UDS_LEN /\\ c0 == spec_hmac dice_hash_alg (spec_hash dice_hash_alg uds_bytes) (spec_hash dice_hash_alg repr.l0_binary)

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



// val hacl_hash (alg:alg_t)
//               (src_len: hashable_len)
//               (src:A.larray U8.t (US.v src_len))
//               (dst:A.larray U8.t (US.v (digest_len alg)))
//               (#psrc:perm)
//               (#src_seq #dst_seq:erased (Seq.seq U8.t))
//   : stt unit
//     (pts_to dst dst_seq **
//      pts_to src #psrc src_seq)
//     (fun _ ->
//        pts_to src #psrc src_seq **
//        pts_to dst (spec_hash alg src_seq))
REQUIRES("dst |-> 'dst_seq")
REQUIRES("src |-> Frac 'p 'src_seq")
ENSURES("src |-> Frac 'p 'src_seq")
ENSURES("exists* dst_seq. dst |-> dst_seq")
void hacl_hash (uint8_t alg, size_t src_len, ISARRAY(src_len)uint8_t *src, ISARRAY(DICE_DIGEST_LEN)uint8_t *dst)
{
    return;
}


// val hacl_hmac (alg:alg_t { alg == Spec.Hash.Definitions.sha2_256 })
//               (dst:A.larray U8.t (US.v (digest_len alg)))
//               (key:A.array U8.t)
//               (key_len: hashable_len { US.v key_len == A.length key })
//               (msg:A.array U8.t)
//               (msg_len: hashable_len { US.v msg_len == A.length msg })
//               (#pkey #pmsg:perm)
//               (#dst_seq:erased (Seq.seq U8.t))
//               (#key_seq:erased (Seq.seq U8.t))
//               (#msg_seq:erased (Seq.seq U8.t))
//   : stt unit
//     (pts_to dst dst_seq **
//      pts_to key #pkey key_seq **
//      pts_to msg #pmsg msg_seq)
//     (fun _ ->
//        pts_to key #pkey key_seq **
//        pts_to msg #pmsg msg_seq **
//        pts_to dst (spec_hmac alg key_seq msg_seq))

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
// ENSURES(freeable a)
ENSURES(pure (length a == UInt64.v len))
uint8_t* new_array(size_t len)
{ LEMMA(admit()); return NULL; }

ERASED_ARG(#repr:erased _)
ERASED_ARG(#p:_)
//REQUIRES(is_engine_record record p repr)
//REQUIRES(exists* s. scratch |-> s)

PRESERVES(is_engine_record record p repr)
RETURNS(b:bool)
//ENSURES(is_engine_record record p repr)
//ENSURES(exists* s. scratch |-> s) //remove scratch parameter and all its uses once array allocation is supported
//Vidush: Removed the scratch parameter since we have support for stack allocated array.
bool authenticate_l0_image(engine_record_t *record/*, ISARRAY(DICE_DIGEST_LEN)uint8_t *scratch*/)
{
    LEMMA(unfold is_engine_record);
    LEMMA(engine_record_t_explode (!record));
    bool valid_header_sig = ed25519_verify(record->l0_image_auth_pubkey, record->l0_image_header, record->l0_image_header_size, record->l0_image_header_sig);
    if (valid_header_sig)
    {   
        uint8_t scratch[DICE_DIGEST_LEN];
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


// fn compute_cdi
//   (cdi:A.larray U8.t (SZ.v (digest_len dice_hash_alg)))
//   (uds:A.larray U8.t (US.v uds_len))
//   ([@@@ Rust_mut_binder] record:engine_record_t)
//   (#uds_perm #p:perm)
//   (#uds_bytes:Ghost.erased (Seq.seq U8.t))
//   requires pts_to uds #uds_perm uds_bytes
//         ** pts_to cdi 'c0
//         ** engine_record_perm record p 'repr
//   returns record:engine_record_t
//   ensures engine_record_perm record p 'repr
//        ** pts_to uds #uds_perm uds_bytes
//        ** (exists* (c1:Seq.seq U8.t). 
//             pts_to cdi c1 **
//             pure (cdi_functional_correctness c1 uds_bytes 'repr))
// {
//   A.pts_to_len uds;
//   let mut uds_digest = [| 0uy; dice_digest_len |];
//   let mut l0_digest = [| 0uy; dice_digest_len |];
//   hacl_hash dice_hash_alg uds_len uds uds_digest;

//   unfold engine_record_perm record p 'repr;

//   V.to_array_pts_to record.l0_binary;
//   hacl_hash dice_hash_alg record.l0_binary_size (V.vec_to_array record.l0_binary) l0_digest;
//   V.to_vec_pts_to record.l0_binary;

//   fold engine_record_perm record p 'repr;

//   hacl_hmac dice_hash_alg cdi 
//     uds_digest dice_digest_len
//     l0_digest dice_digest_len;
  
//   record
// }



ERASED_ARG(#uds_perm:_)
ERASED_ARG(#p:_)
ERASED_ARG(#repr:erased _)
PRESERVES(is_engine_record record p repr)
ERASED_ARG(#uds_bytes:erased _)
PRESERVES("Pulse.Lib.Array.pts_to uds #uds_perm uds_bytes") // ** pts_to cdi 'c0 ** engine_record_perm record p 'repr")
PRESERVES("Pulse.Lib.Array.pts_to cdi 'c0")
RETURNS(ref engine_record_t)
// ENSURES("engine_record_perm record p 'repr \
//        ** pts_to uds #uds_perm uds_bytes \
//        ** (exists* (c1:Seq.seq U8.t). \
//             pts_to cdi c1 ** \
//             pure (cdi_functional_correctness c1 uds_bytes 'repr))")

engine_record_t *compute_cdi(ISARRAY(DICE_DIGEST_LEN)uint8_t *cdi, ISARRAY(UDS_LEN)uint8_t *uds, engine_record_t *record){
    LEMMA(unfold is_engine_record);
    LEMMA(engine_record_t_explode (!record));
    uint8_t uds_digest[DICE_DIGEST_LEN];
    uint8_t l0_digest[DICE_DIGEST_LEN];
    hacl_hash(DICE_HASH_ALG, UDS_LEN, uds, uds_digest);
    hacl_hash(DICE_HASH_ALG, record->l0_binary_size, record->l0_binary, l0_digest);
    LEMMA(engine_record_t_recover (!record));
    LEMMA(fold is_engine_record);

    hacl_hmac(DICE_HASH_ALG, cdi, uds_digest, DICE_DIGEST_LEN, l0_digest, DICE_DIGEST_LEN);
    
    return record;
}



