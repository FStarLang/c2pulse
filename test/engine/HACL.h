#ifndef _HACL_H_
#define _HACL_H_
#include <stdlib.h>
#include <stdint.h>
#include "../include/PulseMacros.h"

ERASED_ARG(#ppubk #phdr #psig:_)
ERASED_ARG(#pubk_seq #hdr_seq #sig_seq:erased (Seq.seq U8.t))
PRESERVES(pubk |-> Frac ppubk pubk_seq)
PRESERVES(hdr |-> Frac phdr hdr_seq)
PRESERVES(sig |-> Frac psig sig_seq)
bool ed25519_verify(ISARRAY() uint8_t *pubk, ISARRAY() uint8_t *hdr, size_t hdr_len, ISARRAY() uint8_t *sig);

REQUIRES("dst |-> 'dst_seq")
REQUIRES("src |-> Frac 'p 'src_seq")
ENSURES("src |-> Frac 'p 'src_seq")
ENSURES("exists* dst_seq. dst |-> dst_seq")
void hacl_hash (uint8_t alg, size_t src_len, ISARRAY(src_len)uint8_t *src, ISARRAY(DICE_DIGEST_LEN)uint8_t *dst);


REQUIRES("dst |-> 'dst_seq")
REQUIRES("key |-> Frac 'p 'key_seq")
REQUIRES("msg |-> Frac 'p 'msg_seq")
ENSURES("key |-> Frac 'p 'key_seq")
ENSURES("msg |-> Frac 'p 'msg_seq")
ENSURES("exists* dst_seq. dst |-> dst_seq")
void hacl_hmac(uint8_t alg, ISARRAY(DICE_DIGEST_LEN)uint8_t *dst, ISARRAY(key_len)uint8_t *key, size_t key_len, ISARRAY(msg_len)uint8_t *msg, size_t msg_len);

RETURNS(SizeT.t)
size_t hashf(uint16_t key);
#endif