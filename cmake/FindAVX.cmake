include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_FLAGS_SAVE ${CMAKE_REQUIRED_FLAGS})

set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

if (NOT DEFINED AVX2_ENABLED)
    set(AVX2_CODE "
        #include <immintrin.h>

        #if !defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)
        extern \"C\" {
            __m256 _ZGVdN8v_logf(__m256 x);
        }
        inline __m256 _mm256_log_ps(__m256 x) noexcept {
                return _ZGVdN8v_logf(x);
        }
        #endif

        int main()
        {
            __m256 a = _mm256_log_ps(_mm256_setzero_ps());
            return 0;
        }
    ")
    check_cxx_source_compiles("${AVX2_CODE}" AVX2_ENABLED)
    set_property(CACHE AVX2_ENABLED PROPERTY TYPE BOOL)
endif()

if (NOT DEFINED AVX512BW_ENABLED)
    set(AVX512BW_CODE "
        #include <immintrin.h>

        #if !defined(__INTEL_COMPILER) && !defined(__INTEL_LLVM_COMPILER)
        extern \"C\" {
            __m512 _ZGVeN16v_logf(__m512 x);
        }
        inline __m512 _mm512_log_ps(__m512 x) noexcept {
            return _ZGVeN16v_logf(x);
        }
        #endif

        int main()
        {
            __m512i a = _mm512_srli_epi16(_mm512_setzero_epi32(), 8);
            __m512 b = _mm512_log_ps(_mm512_setzero_ps());
            return 0;
        }
        ")
    check_cxx_source_compiles("${AVX512BW_CODE}" AVX512BW_ENABLED)
    set_property(CACHE AVX512BW_ENABLED PROPERTY TYPE BOOL)
endif()

set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_SAVE})
unset(CMAKE_REQUIRED_FLAGS_SAVE)
