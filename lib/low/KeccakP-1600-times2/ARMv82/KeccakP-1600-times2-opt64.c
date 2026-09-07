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
#include <KeccakP-1600-SnP.h>
#include "KeccakP-1600-times2-SnP.h"

#define LANE_LENGTH_IN_BYTES 8
#define NO_LENGTH 25
#define NO_STATES 2

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_InitializeAll(KeccakP1600times2_ARMv8Asha3 *states){
    unsigned int i;
    for (i = 0; i < 400; i++){
        ((unsigned char*)(states))[i] = 0;
    }
}

/* ---------------------------------------------------------------- */

// HELPER FUNCTION: given an offset and state index, return index position in bytes
static size_t byteIndex(unsigned int instanceIndex, unsigned int offset){
    return 16 * (offset / 8) + 8 * instanceIndex + (offset % 8);
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_AddByte(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned char byte, unsigned int offset){
    ((unsigned char *)states)[byteIndex(instanceIndex, offset)] ^= byte; 
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_AddBytes(KeccakP1600times2_ARMv8Asha3 *states,unsigned int instanceIndex, const unsigned char *data, unsigned int offset, unsigned int length){
    unsigned int i;
    for (i = 0; i < length; i++) {
        KeccakP1600times2_opt64_AddByte(states, instanceIndex, data[i], offset + i);
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_AddLanesAll(KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *data, unsigned int laneCount, unsigned int laneOffset){
    unsigned int lane;
    for (lane = 0; lane < laneCount; lane++) {
        KeccakP1600times2_opt64_AddBytes(states, 0, data + 8 * lane, 8 * lane, 8);
        KeccakP1600times2_opt64_AddBytes(states, 1, data + 8 * (laneOffset + lane), 8 * lane, 8);
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_OverwriteBytes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *data, unsigned int offset, unsigned int length){
    unsigned int i;
    for (i = 0; i < length; i++) {
        size_t index = byteIndex(instanceIndex, offset + i);
        ((unsigned char *)states)[index] = data[i];
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_OverwriteLanesAll(KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *data, unsigned int laneCount, unsigned int laneOffset){
    unsigned int lane;
    for (lane = 0; lane < laneCount; lane++){
        KeccakP1600times2_opt64_OverwriteBytes(states, 0, data + 8 * lane, 8 * lane, 8);
        KeccakP1600times2_opt64_OverwriteBytes(states, 1, data + 8 * (laneOffset + lane), 8 * lane, 8);
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_OverwriteWithZeroes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned int byteCount){
    unsigned int bytePos;
    for (bytePos = 0; bytePos < byteCount; bytePos++) {
        size_t index = byteIndex(instanceIndex, bytePos);
        ((unsigned char *)states)[index] = 0;
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_ExtractBytes(const KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned char *data, unsigned int offset, unsigned int length){
    unsigned int i;
    const unsigned char *stateBytes = (const unsigned char *)states;
    for (i = 0; i < length; i++) {
        size_t index = byteIndex(instanceIndex, offset + i);
        data[i] = stateBytes[index];
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_ExtractAndAddBytes(const KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *input, unsigned char *output, unsigned int offset, unsigned int length){
    unsigned int i;
    const unsigned char *stateBytes = (const unsigned char *)states;
    for (i = 0; i < length; i++) {
        size_t index = byteIndex(instanceIndex, offset + i);
        output[i] = input[i] ^ stateBytes[index]; // XOR == add
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_ExtractLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, unsigned char *data, unsigned int laneCount, unsigned int laneOffset){
    unsigned int lane;
    for (lane = 0; lane < laneCount; lane++){
        KeccakP1600times2_opt64_ExtractBytes(states, 0, data + 8 * lane, 8 * lane, 8);
        KeccakP1600times2_opt64_ExtractBytes(states, 1, data + 8 * (laneOffset + lane), 8 * lane, 8);
    }
}

/* ---------------------------------------------------------------- */

void KeccakP1600times2_opt64_ExtractAndAddLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *input, unsigned char *output, unsigned int laneCount, unsigned int laneOffset){
    unsigned int lane;
    for (lane = 0; lane < laneCount; lane++) {
        KeccakP1600times2_opt64_ExtractAndAddBytes(states, 0, input + 8 * lane, output + 8 * lane, 8 * lane, 8);
        KeccakP1600times2_opt64_ExtractAndAddBytes(states, 1, input + 8 * (laneOffset + lane), output + 8 * (laneOffset + lane), 8 * lane, 8);
    }
}



