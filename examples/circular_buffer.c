#include "pal.h"
#include <stdint.h>
#include <string.h>

// Flat verified circular buffer state.
// Buffer memory is externally owned (by QUIC_RECV_CHUNK).
typedef struct VERIFIED_CIRC_BUFFER {
    _array
    uint8_t*  Buffer;        // Physical buffer (externally owned, not freed by us)
    uint32_t  ReadStart;     // Read head position in physical buffer
    uint32_t  AllocLength;   // Physical buffer size (always a power of 2)
    uint32_t  PrefixLength;  // Length of contiguous readable data from ReadStart
    uint32_t  VirtualLength; // Maximum allowed buffer size (always a power of 2)
} VERIFIED_CIRC_BUFFER;

// Linearize (copy) the circular buffer contents into a new destination buffer,
// unwrapping the circular layout into a linear one.
// Copies: dst[0..head_len) = src[ReadStart..AllocLength)
//         dst[head_len..AllocLength) = src[0..ReadStart)
//
// Copies the circular buffer contents into a linear destination buffer,
// unwrapping the wrap-around. This is the two-memcpy form proven equivalent
// to the byte-by-byte loop by Spec.linearized_phys_two_segments.
//
// Pre:  DstLength >= Buf->AllocLength
// Post: dst[0..al-rs) = src[rs..al), dst[al-rs..al) = src[0..rs)
//       (remaining dst bytes are untouched / zero-initialized by caller)
void
VerifiedCircBufLinearizeTo(
    VERIFIED_CIRC_BUFFER* Buf,
    uint8_t* DstBuffer,
    uint32_t DstLength)
{
    uint32_t rs = Buf->ReadStart;
    uint32_t al = Buf->AllocLength;
    uint32_t head_len = al - rs;

    if (head_len > 0) {
        memcpy(DstBuffer, Buf->Buffer + rs, head_len);
    }
    if (rs > 0) {
        memcpy(DstBuffer + head_len, Buf->Buffer, rs);
    }
}

// Update the verified buffer state after a resize operation.
// Points Buffer at the new (linearized) allocation and resets ReadStart to 0.
//
// After linearization, update the verified buffer to point at the new
// (linear) allocation with ReadStart reset to 0.
void
VerifiedCircBufSyncAfterResize(
    VERIFIED_CIRC_BUFFER* Buf,
    uint8_t* NewBuffer,
    uint32_t NewAllocLength)
{
    Buf->Buffer = NewBuffer;
    Buf->ReadStart = 0;
    Buf->AllocLength = NewAllocLength;
}

// Drain (consume) DrainLength bytes from the front of the buffer.
// Advances ReadStart circularly and decreases PrefixLength.
//
// Advances the read head by DrainLength positions (modular) and
// decreases PrefixLength accordingly.
//
// Pre:  DrainLength <= PrefixLength
//       DrainLength <= AllocLength
// Post: ReadStart' = (ReadStart + DrainLength) % AllocLength
//       PrefixLength' = PrefixLength - DrainLength
void
VerifiedCircBufDrain(
    VERIFIED_CIRC_BUFFER* Buf,
    uint32_t DrainLength)
{
    uint32_t rs = Buf->ReadStart;
    uint32_t al = Buf->AllocLength;
    uint32_t pl = Buf->PrefixLength;

    uint32_t temp = rs + DrainLength;
    uint32_t new_rs = temp % al;
    Buf->ReadStart = new_rs;

    uint32_t new_pl = pl - DrainLength;
    Buf->PrefixLength = new_pl;
}