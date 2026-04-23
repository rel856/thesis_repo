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

#ifndef _KeccakP_1600_SnP_h_
#define _KeccakP_1600_SnP_h_

// #define SnP_Permute KeccakP1600_Permute_12rounds // omit (nonexistent) 24-rounds for now, remove later

#include <stdint.h>
#include "align.h"
#include "PlSnP-common.h"

// State for SnP 
typedef struct {
    ALIGN(64) uint64_t A[25][2];
} KeccakP1600times2_ARMv8Asha3;

typedef KeccakP1600times2_ARMv8Asha3 KeccakP1600_state;

// const char *KeccakP1600_GetImplementation(); // call from runtime Dispatch instead of defining here
#define KeccakP1600times2_GetFeatures()                   (SnP_Feature_Main)

/* Keccak-p[1600] */

#define KeccakP1600_stateSizeInBytes    200
#define KeccakP1600_stateAlignment      8
// #define KeccakP1600_12rounds_FastLoop_supported

void KeccakP1600times2_StaticInitialize( void );
void KeccakP1600times2_InitializeAll(KeccakP1600times2_ARMv8Asha3 *state);
void KeccakP1600times2_AddByte(KeccakP1600times2_ARMv8Asha3 *state, unsigned char data, unsigned int offset);
void KeccakP1600times2_AddBytes(KeccakP1600times2_ARMv8Asha3 *state, const unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP1600times2_OverwriteBytes(KeccakP1600times2_ARMv8Asha3 *state, const unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP1600times2_OverwriteLanesAll(KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *data, unsigned int laneCount, unsigned int laneOffset);
void KeccakP1600times2_OverwriteWithZeroes(KeccakP1600times2_ARMv8Asha3 *state, unsigned int byteCount);
void KeccakP1600times2_PermuteAll_4rounds(KeccakP1600times2_ARMv8Asha3 *states);
void KeccakP1600times2_PermuteAll_6rounds(KeccakP1600times2_ARMv8Asha3 *states);
void KeccakP1600times2_PermuteAll_12rounds(KeccakP1600times2_ARMv8Asha3 *states);
void KeccakP1600times2_PermuteAll_24rounds(KeccakP1600times2_ARMv8Asha3 *states);
void KeccakP1600times2_ExtractBytes(const KeccakP1600times2_ARMv8Asha3 *state, unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP1600times2_ExtractLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, unsigned char *data, unsigned int laneCount, unsigned int laneOffset);
void KeccakP1600times2_ExtractAndAddBytes(const KeccakP1600times2_ARMv8Asha3 *state, const unsigned char *input, unsigned char *output, unsigned int offset, unsigned int length);
void KeccakP1600times2_ExtractAndAddLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *input, unsigned char *output, unsigned int laneCount, unsigned int laneOffset);

// size_t KeccakP1600_12rounds_FastLoop_Absorb(KeccakP1600_ARMv8Asha3_times2 *state, unsigned int laneCount, const unsigned char *data, size_t dataByteLen);

// Keccak-p1600 x2


// /* Keccak-p[1600]×2 */
// int KeccakP1600times2_IsAvailable();
// const char * KeccakP1600times2_GetImplementation();
// void KeccakP1600times2_Permute_12rounds(void *state);
// void KT128_Process2Leaves(const unsigned char *input, unsigned char *output);
// void KT256_Process2Leaves(const unsigned char *input, unsigned char *output);

// /* Keccak-p[1600]×4 */

// int KeccakP1600times4_IsAvailable();
// const char * KeccakP1600times4_GetImplementation();

// /* Keccak-p[1600]×8 */

// int KeccakP1600times8_IsAvailable();
// const char * KeccakP1600times8_GetImplementation();

#define KeccakF1600_FastLoop_Absorb(...)                0
#define KeccakP1600_12rounds_FastLoop_Absorb(...)       0
#define KeccakP1600_ODDuplexingFastInOut(...)           0
#define KeccakP1600_12rounds_ODDuplexingFastInOut(...)  0
#define KeccakP1600_ODDuplexingFastOut(...)             0
#define KeccakP1600_12rounds_ODDuplexingFastOut(...)    0
#define KeccakP1600_ODDuplexingFastIn(...)              0
#define KeccakP1600_12rounds_ODDuplexingFastIn(...)     0

#endif
