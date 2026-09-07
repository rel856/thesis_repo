/*
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

ARM CPU feature detection adapted from libaegis by Frank Denis.
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "KeccakP-1600-times2-SnP.h"

#ifdef KeccakP1600_disableParallelism
#undef KeccakP1600_enable_simd_options
#else

// Forward declarations
void SetArmProcessorCapabilities();

#ifdef KeccakP1600_enable_simd_options
int NEON_requested_disabled = 0;
int ARM_SHA3_requested_disabled = 0;
#endif  // KeccakP1600_enable_simd_options

int enableNEON = 0;
int enableARM_SHA3 = 0;

/* ---------------------------------------------------------------- */
/* Platform-specific includes for CPU feature detection */
/* ---------------------------------------------------------------- */

#if defined(__linux__) && (defined(__aarch64__) || defined(__arm__))
#define HAVE_LINUX_ARM
#if defined(__GLIBC__) || defined(__BIONIC__)
#include <sys/auxv.h>
#define HAVE_GETAUXVAL
#endif
#endif

#if defined(__APPLE__) && (defined(__aarch64__) || defined(__arm__))
#define HAVE_APPLE_ARM
#include <sys/sysctl.h>
#endif

#if defined(_WIN32) && (defined(_M_ARM64) || defined(_M_ARM))
#define HAVE_WINDOWS_ARM
#include <windows.h>
#endif

#if defined(__ANDROID__) && (defined(__aarch64__) || defined(__arm__))
#define HAVE_ANDROID_ARM
#include <cpu-features.h>
#endif

/* ---------------------------------------------------------------- */
/* Hardware capability constants */
/* ---------------------------------------------------------------- */

// 32-bit ARM hwcaps (AT_HWCAP)
#ifndef ARM_HWCAP_NEON
#define ARM_HWCAP_NEON (1L << 12)
#endif

// AArch64 hwcaps (AT_HWCAP)
#ifndef AARCH64_HWCAP_ASIMD
#define AARCH64_HWCAP_ASIMD (1L << 1)
#endif

#ifndef AARCH64_HWCAP_SHA3
#define AARCH64_HWCAP_SHA3 (1L << 17)
#endif

/* ---------------------------------------------------------------- */
/* CPU feature detection */
/* ---------------------------------------------------------------- */

enum arm_cpu_feature {
    ARM_NEON = 1 << 0,
    ARM_SHA3 = 1 << 1,
    ARM_UNDEFINED = 1 << 30
};

static enum arm_cpu_feature g_arm_cpu_features = ARM_UNDEFINED;

#if defined(HAVE_LINUX_ARM) && defined(HAVE_GETAUXVAL)
static int _have_hwcap(unsigned long hwcap_bit) {
    unsigned long hwcap = getauxval(AT_HWCAP);
    return (hwcap & hwcap_bit) != 0;
}
#endif

#if defined(HAVE_APPLE_ARM)
static int _have_arm_feature(const char *feature_name) {
    int64_t feature_present = 0;
    size_t size = sizeof(feature_present);
    if (sysctlbyname(feature_name, &feature_present, &size, NULL, 0) != 0) {
        return 0;
    }
    return feature_present != 0;
}
#endif

static enum arm_cpu_feature get_arm_cpu_features(void) {
    if (g_arm_cpu_features != ARM_UNDEFINED) {
        return g_arm_cpu_features;
    }

    enum arm_cpu_feature features = 0;

    /* ---------------------------------------------------------------- */
    /* NEON Detection */
    /* ---------------------------------------------------------------- */

    // Compile-time check - if built with NEON, assume available
#if defined(__ARM_NEON) || defined(__aarch64__) || defined(_M_ARM64)
    features |= ARM_NEON;
#elif defined(HAVE_LINUX_ARM) && defined(HAVE_GETAUXVAL)
    // Runtime detection on Linux ARM
#if defined(__aarch64__)
    if (_have_hwcap(AARCH64_HWCAP_ASIMD)) {
        features |= ARM_NEON;
    }
#elif defined(__arm__)
    if (_have_hwcap(ARM_HWCAP_NEON)) {
        features |= ARM_NEON;
    }
#endif
#elif defined(HAVE_ANDROID_ARM)
    // Android detection
    uint64_t android_features = android_getCpuFeatures();
    if (android_features & ANDROID_CPU_ARM_FEATURE_NEON) {
        features |= ARM_NEON;
    }
#elif defined(HAVE_WINDOWS_ARM)
    // Windows ARM64 - assume all have NEON
    features |= ARM_NEON;
#endif

    /* ---------------------------------------------------------------- */
    /* SHA3 Detection (requires NEON) */
    /* ---------------------------------------------------------------- */

    if (features & ARM_NEON) {
        // Compile-time check
#if defined(__ARM_FEATURE_SHA3)
        features |= ARM_SHA3;
#elif defined(HAVE_LINUX_ARM) && defined(HAVE_GETAUXVAL) && defined(__aarch64__)
        // Runtime detection on Linux AArch64
        if (_have_hwcap(AARCH64_HWCAP_SHA3)) {
            features |= ARM_SHA3;
        }
#elif defined(HAVE_APPLE_ARM)
        // macOS/Apple Silicon detection
        if (_have_arm_feature("hw.optional.arm.FEAT_SHA3")) {
            features |= ARM_SHA3;
        }
#endif
    }

    g_arm_cpu_features = features;
    return features;
}

void SetArmProcessorCapabilities() {
    enum arm_cpu_feature features = get_arm_cpu_features();
    enableNEON = (features & ARM_NEON) != 0;
    enableARM_SHA3 = (features & ARM_SHA3) != 0;
}

#ifdef KeccakP1600_enable_simd_options
    enableNEON = enableNEON && !NEON_requested_disabled;
    enableARM_SHA3 = enableARM_SHA3 && !ARM_SHA3_requested_disabled;
#endif  // KeccakP1600_enable_simd_options

/* ---------------------------------------------------------------- */
/* External function declarations */
/* ---------------------------------------------------------------- */
// Generic ARM64 implementations from KeccakP-1600-times2-opt64.c 
extern void KeccakP1600times2_opt64_InitializeAll(KeccakP1600times2_ARMv8Asha3 *states);
extern void KeccakP1600times2_opt64_AddByte(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned char data, unsigned int offset);
extern void KeccakP1600times2_opt64_AddBytes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *data, unsigned int offset, unsigned int length);
extern void KeccakP1600times2_opt64_AddLanesAll(KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *data, unsigned int laneCount, unsigned int laneOffset);
extern void KeccakP1600times2_opt64_OverwriteBytes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *data, unsigned int offset, unsigned int length);
extern void KeccakP1600times2_opt64_OverwriteLanesAll(KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *data, unsigned int laneCount, unsigned int laneOffset);
extern void KeccakP1600times2_opt64_OverwriteWithZeroes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned int byteCount);
extern void KeccakP1600times2_opt64_ExtractBytes(const KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned char *data, unsigned int offset, unsigned int length);
extern void KeccakP1600times2_opt64_ExtractLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, unsigned char *data, unsigned int laneCount, unsigned int laneOffset);
extern void KeccakP1600times2_opt64_ExtractAndAddBytes(const KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *input, unsigned char *output, unsigned int offset, unsigned int length);
extern void KeccakP1600times2_opt64_ExtractAndAddLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *input, unsigned char *output, unsigned int laneCount, unsigned int laneOffset);

// permutation functions (from assembly)
extern void KeccakP1600times2_ARMv8Asha3_PermuteAll_4rounds(KeccakP1600times2_ARMv8Asha3 *states);
extern void KeccakP1600times2_ARMv8Asha3_PermuteAll_6rounds(KeccakP1600times2_ARMv8Asha3 *states);
extern void KeccakP1600times2_ARMv8Asha3_PermuteAll_12rounds(KeccakP1600times2_ARMv8Asha3 *states);
extern void KeccakP1600times2_ARMv8Asha3_PermuteAll_24rounds(KeccakP1600times2_ARMv8Asha3 *states);

/* ---------------------------------------------------------------- */
/* Dispatch functions for Keccak-p[1600]×2 */
/* ---------------------------------------------------------------- */

int KeccakP1600times2_IsAvailable(void) {
    SetArmProcessorCapabilities();
    if (enableARM_SHA3) {
        return 1;
    }
    else {
        return 0;
    }
}

int KeccakP1600times2_GetFeatures(void) {
    SetArmProcessorCapabilities();
    if (enableARM_SHA3) {
        return PlSnP_Feature_Main;
    }
    else {
        return 0;
    }
}

const char *KeccakP1600times2_GetImplementation(void) {
    SetArmProcessorCapabilities();
    if (enableARM_SHA3) {
        return "ARMv8-A+SHA3 optimized implementation";
    }
    else {
        return "";
    }
}

void KeccakP1600times2_StaticInitialize(void) {
    SetArmProcessorCapabilities();
}

void KeccakP1600times2_InitializeAll(KeccakP1600times2_ARMv8Asha3 *states) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_InitializeAll(states);
}

void KeccakP1600times2_AddByte(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned char data, unsigned int offset) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_AddByte(states, instanceIndex, data, offset);
}

void KeccakP1600times2_AddBytes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *data, unsigned int offset, unsigned int length) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_AddBytes(states, instanceIndex, data, offset, length);
}

void KeccakP1600times2_AddLanesAll(KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *data, unsigned int laneCount, unsigned int laneOffset) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_AddLanesAll(states, data, laneCount, laneOffset);
}

void KeccakP1600times2_OverwriteBytes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *data, unsigned int offset, unsigned int length) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_OverwriteBytes(states, instanceIndex, data, offset, length);
}

void KeccakP1600times2_OverwriteLanesAll(KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *data, unsigned int laneCount, unsigned int laneOffset) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_OverwriteLanesAll(states, data, laneCount, laneOffset);
}

void KeccakP1600times2_OverwriteWithZeroes(KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned int byteCount) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_OverwriteWithZeroes(states, instanceIndex, byteCount);
}

void KeccakP1600times2_ExtractBytes(const KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, unsigned char *data, unsigned int offset, unsigned int length) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_ExtractBytes(states, instanceIndex, data, offset, length);
}

void KeccakP1600times2_ExtractLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, unsigned char *data, unsigned int laneCount, unsigned int laneOffset) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_ExtractLanesAll(states, data, laneCount, laneOffset);
}

void KeccakP1600times2_ExtractAndAddBytes(const KeccakP1600times2_ARMv8Asha3 *states, unsigned int instanceIndex, const unsigned char *input, unsigned char *output, unsigned int offset, unsigned int length) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_ExtractAndAddBytes(states, instanceIndex, input, output, offset, length);
}

void KeccakP1600times2_ExtractAndAddLanesAll(const KeccakP1600times2_ARMv8Asha3 *states, const unsigned char *input, unsigned char *output, unsigned int laneCount, unsigned int laneOffset) {
    SetArmProcessorCapabilities();
    KeccakP1600times2_opt64_ExtractAndAddLanesAll(states, input, output, laneCount, laneOffset);
}

void KeccakP1600times2_PermuteAll_4rounds(KeccakP1600times2_ARMv8Asha3 *states) {
    SetArmProcessorCapabilities();
    if (enableARM_SHA3) {
        KeccakP1600times2_ARMv8Asha3_PermuteAll_4rounds(states);
    }
}

void KeccakP1600times2_PermuteAll_6rounds(KeccakP1600times2_ARMv8Asha3 *states) {
    SetArmProcessorCapabilities();
    if (enableARM_SHA3) {
        KeccakP1600times2_ARMv8Asha3_PermuteAll_6rounds(states);
    }
}

void KeccakP1600times2_PermuteAll_12rounds(KeccakP1600times2_ARMv8Asha3 *states) {
    SetArmProcessorCapabilities();
    if (enableARM_SHA3) {
        KeccakP1600times2_ARMv8Asha3_PermuteAll_12rounds(states);
    }
}

void KeccakP1600times2_PermuteAll_24rounds(KeccakP1600times2_ARMv8Asha3 *states) {
    SetArmProcessorCapabilities();
    if (enableARM_SHA3) {
        KeccakP1600times2_ARMv8Asha3_PermuteAll_24rounds(states);
    }
}


/* ---------------------------------------------------------------- */
/* Optional API for disabling CPU features */
/* ---------------------------------------------------------------- */

#ifdef KeccakP1600_enable_simd_options

int KangarooTwelve_DisableNeon(void) {
    SetArmProcessorCapabilities();
    NEON_requested_disabled = 1;
    if (enableNEON) {
        SetArmProcessorCapabilities();
        return 1;  // NEON was disabled on this call.
    } else {
        return 0;  // Nothing changed.
    }
}

int KangarooTwelve_DisableArmSha3(void) {
    SetArmProcessorCapabilities();
    ARM_SHA3_requested_disabled = 1;
    if (enableARM_SHA3) {
        SetArmProcessorCapabilities();
        return 1;  // ARM SHA3 was disabled on this call.
    } else {
        return 0;  // Nothing changed.
    }
}

void KangarooTwelve_EnableAllArmCpuFeatures(void) {
    NEON_requested_disabled = 0;
    ARM_SHA3_requested_disabled = 0;
    SetArmProcessorCapabilities();
}

#endif  // KeccakP1600_enable_simd_options

#endif  // !KeccakP1600_disableParallelism
