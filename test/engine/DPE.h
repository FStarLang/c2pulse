#ifndef _DPE_H_
#define _DPE_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../include/PulseMacros.h"

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

typedef struct _l1_context_t {
  ISARRAY(32) uint8_t *deviceID_pub;
  ISARRAY(32) uint8_t *aliasKey_priv;
  ISARRAY(32) uint8_t *aliasKey_pub;
  size_t deviceIDCSR_len;
  ISARRAY(deviceIDCSR_len) uint8_t *deviceIDCSR;
  size_t aliasKeyCRT_len;
  ISARRAY(aliasKeyCRT_len) uint8_t *aliasKeyCRT;
} l1_context_t;

#define ENGINE_CONTEXT 0
#define L0_CONTEXT 1
#define L1_CONTEXT 2

typedef union _u_context_t {
  ISARRAY(UDS_LEN) uint8_t *uds;
  ISARRAY(DICE_DIGEST_LEN) uint8_t *cdi;
  l1_context_t l1_context;
} u_context_t;

typedef struct _context_t {
  uint8_t tag;
  u_context_t payload;
} context_t;
#endif