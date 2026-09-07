/*
K12 based on the eXtended Keccak Code Package (XKCP)
https://github.com/XKCP/XKCP

The Keccak-p permutations, designed by Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche.

Implementation by Gilles Van Assche and Ronny Van Keer, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to the Keccak Team website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/

---

Please refer to the XKCP for more details.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "KeccakP-1600-SnP.h"

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_Initialize(KeccakP1600_ARMv8Asha3 *state)
{
    memset(state, 0, 200);
}

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_AddBytesInLane(KeccakP1600_ARMv8Asha3 *state, unsigned int lanePosition, const unsigned char *data, unsigned int offset, unsigned int length)
{
    uint64_t lane;

    if (length == 0)
        return;
    if (length == 1)
        lane = data[0];
    else {
        lane = 0;
        memcpy(&lane, data, length);
    }
    lane <<= offset*8;
    ((uint64_t*)state)[lanePosition] ^= lane;
}

/* ---------------------------------------------------------------- */

static void KeccakP1600_opt64_AddLanes(KeccakP1600_ARMv8Asha3 *state, const unsigned char *data, unsigned int laneCount)
{
    unsigned int i = 0;

    for( ; (i+8)<=laneCount; i+=8) {
        ((uint64_t*)state)[i+0] ^= ((uint64_t*)data)[i+0];
        ((uint64_t*)state)[i+1] ^= ((uint64_t*)data)[i+1];
        ((uint64_t*)state)[i+2] ^= ((uint64_t*)data)[i+2];
        ((uint64_t*)state)[i+3] ^= ((uint64_t*)data)[i+3];
        ((uint64_t*)state)[i+4] ^= ((uint64_t*)data)[i+4];
        ((uint64_t*)state)[i+5] ^= ((uint64_t*)data)[i+5];
        ((uint64_t*)state)[i+6] ^= ((uint64_t*)data)[i+6];
        ((uint64_t*)state)[i+7] ^= ((uint64_t*)data)[i+7];
    }
    for( ; (i+4)<=laneCount; i+=4) {
        ((uint64_t*)state)[i+0] ^= ((uint64_t*)data)[i+0];
        ((uint64_t*)state)[i+1] ^= ((uint64_t*)data)[i+1];
        ((uint64_t*)state)[i+2] ^= ((uint64_t*)data)[i+2];
        ((uint64_t*)state)[i+3] ^= ((uint64_t*)data)[i+3];
    }
    for( ; (i+2)<=laneCount; i+=2) {
        ((uint64_t*)state)[i+0] ^= ((uint64_t*)data)[i+0];
        ((uint64_t*)state)[i+1] ^= ((uint64_t*)data)[i+1];
    }
    if (i<laneCount) {
        ((uint64_t*)state)[i+0] ^= ((uint64_t*)data)[i+0];
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_AddByte(KeccakP1600_ARMv8Asha3 *state, unsigned char byte, unsigned int offset)
{
    ((unsigned char*)(state))[offset] ^= byte;
}

/* ---------------------------------------------------------------- */

#define SnP_AddBytes(state, data, offset, length, SnP_AddLanes, SnP_AddBytesInLane, SnP_laneLengthInBytes) \
    { \
        if ((offset) == 0) { \
            SnP_AddLanes(state, data, (length)/SnP_laneLengthInBytes); \
            SnP_AddBytesInLane(state, \
                (length)/SnP_laneLengthInBytes, \
                (data)+((length)/SnP_laneLengthInBytes)*SnP_laneLengthInBytes, \
                0, \
                (length)%SnP_laneLengthInBytes); \
        } \
        else { \
            unsigned int _sizeLeft = (length); \
            unsigned int _lanePosition = (offset)/SnP_laneLengthInBytes; \
            unsigned int _offsetInLane = (offset)%SnP_laneLengthInBytes; \
            const unsigned char *_curData = (data); \
            while(_sizeLeft > 0) { \
                unsigned int _bytesInLane = SnP_laneLengthInBytes - _offsetInLane; \
                if (_bytesInLane > _sizeLeft) \
                    _bytesInLane = _sizeLeft; \
                SnP_AddBytesInLane(state, _lanePosition, _curData, _offsetInLane, _bytesInLane); \
                _sizeLeft -= _bytesInLane; \
                _lanePosition++; \
                _offsetInLane = 0; \
                _curData += _bytesInLane; \
            } \
        } \
    }

void KeccakP1600_opt64_AddBytes(KeccakP1600_ARMv8Asha3 *state, const unsigned char *data, unsigned int offset, unsigned int length)
{
    SnP_AddBytes(state, data, offset, length, KeccakP1600_opt64_AddLanes, KeccakP1600_opt64_AddBytesInLane, 8);
}

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_ExtractBytesInLane(const KeccakP1600_ARMv8Asha3 *state, unsigned int lanePosition, unsigned char *data, unsigned int offset, unsigned int length)
{
    uint64_t lane;

        if (length == 0)
            return;

        lane = state->A[lanePosition];
        memcpy(data, ((unsigned char *)&lane) + offset, length);
}

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_ExtractLanes(const KeccakP1600_ARMv8Asha3 *state, unsigned char *data, unsigned int laneCount)
{
    memcpy(data, state, laneCount*8);
}

/* ---------------------------------------------------------------- */

#define SnP_ExtractBytes(state, data, offset, length, SnP_ExtractLanes, SnP_ExtractBytesInLane, SnP_laneLengthInBytes) \
    { \
        if ((offset) == 0) { \
            SnP_ExtractLanes(state, data, (length)/SnP_laneLengthInBytes); \
            SnP_ExtractBytesInLane(state, \
                (length)/SnP_laneLengthInBytes, \
                (data)+((length)/SnP_laneLengthInBytes)*SnP_laneLengthInBytes, \
                0, \
                (length)%SnP_laneLengthInBytes); \
        } \
        else { \
            unsigned int _sizeLeft = (length); \
            unsigned int _lanePosition = (offset)/SnP_laneLengthInBytes; \
            unsigned int _offsetInLane = (offset)%SnP_laneLengthInBytes; \
            unsigned char *_curData = (data); \
            while(_sizeLeft > 0) { \
                unsigned int _bytesInLane = SnP_laneLengthInBytes - _offsetInLane; \
                if (_bytesInLane > _sizeLeft) \
                    _bytesInLane = _sizeLeft; \
                SnP_ExtractBytesInLane(state, _lanePosition, _curData, _offsetInLane, _bytesInLane); \
                _sizeLeft -= _bytesInLane; \
                _lanePosition++; \
                _offsetInLane = 0; \
                _curData += _bytesInLane; \
            } \
        } \
    }
    
/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_ExtractBytes(const KeccakP1600_ARMv8Asha3 *state, unsigned char *data, unsigned int offset, unsigned int length)
{
    SnP_ExtractBytes(state, data, offset, length, KeccakP1600_opt64_ExtractLanes, KeccakP1600_opt64_ExtractBytesInLane, 8);
}

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_OverwriteBytes(KeccakP1600_ARMv8Asha3 *state, const unsigned char *data, unsigned int offset, unsigned int length){
    unsigned int i;
    for (i = 0; i < length; i++) {
        ((unsigned char*)(state))[offset + i] = data[i];
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_OverwriteWithZeroes(KeccakP1600_ARMv8Asha3 *state, unsigned int byteCount){
    unsigned int i;
    for (i = 0; i < byteCount; i++) {
        ((unsigned char*)(state))[i] = 0;
    }
}

/* ---------------------------------------------------------------- */

//taken from readable KECCAK
static uint64_t load64(const unsigned char *data){
    uint64_t x;
    memcpy(&x, data, sizeof(x));
    return x;
}

/* ---------------------------------------------------------------- */

void KeccakP1600_opt64_ExtractAndAddBytes(const KeccakP1600_ARMv8Asha3 *state, const unsigned char *input, unsigned char *output, unsigned int offset, unsigned int length){
    const unsigned char *stateBytes = (const unsigned char *)state;
    unsigned int i;
    for (i = 0; i < length; ++i)
        output[i] = input[i] ^ stateBytes[offset + i];
}
