#pragma once

#include <opencv2/opencv.hpp>
#include <cstdint>

// SIMD优化类
class SIMDOptimization
{
public:
    // x86 SSE2 实现
    static int32_t IM_Conv_SIMD_SSE2(uint8_t* pCharKernel, uint8_t* pCharConv, int iLength);
    
    // ARM NEON 实现
    static int32_t IM_Conv_SIMD_NEON(uint8_t* pCharKernel, uint8_t* pCharConv, int iLength);
    
    // 通用实现（无SIMD）
    static int32_t IM_Conv_Generic(uint8_t* pCharKernel, uint8_t* pCharConv, int iLength);
    
    // 根据平台自动选择最优实现
    static int32_t IM_Conv_SIMD(uint8_t* pCharKernel, uint8_t* pCharConv, int iLength);
    
    // 检测平台支持
    static bool isSSE2Supported();
    static bool isNEONSupported();
    
private:
    // SSE2 水平求和
    static inline int32_t hsum_epi32_sse2(void* x);
    
    // NEON 水平求和
    static inline int32_t vaddvq_s32(void* v);
};

// 平台检测宏
#ifdef __SSE2__
    #define SSE2_AVAILABLE 1
#else
    #define SSE2_AVAILABLE 0
#endif

#ifdef __ARM_NEON
    #define NEON_AVAILABLE 1
#else
    #define NEON_AVAILABLE 0
#endif 