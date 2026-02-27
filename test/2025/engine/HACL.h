#ifndef _HACL_H_
#define _HACL_H_
#include <stdlib.h>
#include <stdint.h>
#include "../c2pulse.h"

_preserves((_slprop) _inline_pulse(pubk |-> Frac ppubk pubk_seq))
_preserves((_slprop) _inline_pulse(hdr |-> Frac phdr hdr_seq))
_preserves((_slprop) _inline_pulse(sig |-> Frac psig sig_seq))
bool ed25519_verify(uint8_t *pubk, uint8_t *hdr, size_t hdr_len, uint8_t *sig);

_requires((_slprop) _inline_pulse(dst |-> 'dst_seq))
_requires((_slprop) _inline_pulse(src |-> Frac 'p 'src_seq))
_ensures((_slprop) _inline_pulse(src |-> Frac 'p 'src_seq))
_ensures((_slprop) _inline_pulse(exists* dst_seq. dst |-> dst_seq))
void hacl_hash (uint8_t alg, size_t src_len, uint8_t *src, uint8_t *dst);

_requires((_slprop) _inline_pulse(dst |-> 'dst_seq))
_requires((_slprop) _inline_pulse(key |-> Frac 'p 'key_seq))
_requires((_slprop) _inline_pulse(msg |-> Frac 'p 'msg_seq))
_ensures((_slprop) _inline_pulse(key |-> Frac 'p 'key_seq))
_ensures((_slprop) _inline_pulse(msg |-> Frac 'p 'msg_seq))
_ensures((_slprop) _inline_pulse(exists* dst_seq. dst |-> dst_seq))
void hacl_hmac(uint8_t alg, uint8_t *dst, uint8_t *key, size_t key_len, uint8_t *msg, size_t msg_len);

size_t hashf(uint16_t key);
#endif