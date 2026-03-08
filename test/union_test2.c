#include "c2pulse.h"
#include <stdint.h>

typedef uint8_t *uds_array
    _refine(this._length == 32) _array;

typedef uint8_t *dice_digest
    _refine(this._length == 64) _array;

typedef union _u_context_t {
  uds_array uds;
  dice_digest cdi;
} u_context_t;

typedef struct {
  u_context_t payload;
} context_t;

// TODO
#define _is_uds(ctx) ((bool) _inline_pulse(Field__u_context_t__uds? $(ctx)))

void test1(u_context_t ctx)
    _requires(_is_uds(ctx))
{
    uint8_t *uds_buf = ctx.uds;
}

void test2(u_context_t *ctx)
    _requires(_is_uds(*ctx))
{
    uint8_t *uds_buf = ctx->uds;
}

void test3a(context_t ctx)
    _requires(_is_uds(ctx.payload))
{
    u_context_t *payload = &ctx.payload;
    uint8_t *uds_buf = payload->uds;
}


void test3(context_t ctx)
    _requires(_is_uds(ctx.payload))
{
    uint8_t *uds_buf = ctx.payload.uds;
}

void test4(context_t *ctx)
    _requires(_is_uds(ctx->payload))
{
    uint8_t *uds_buf = ctx->payload.uds;
}