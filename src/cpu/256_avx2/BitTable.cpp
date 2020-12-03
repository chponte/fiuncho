#include <cmath>
#include <fiuncho/engine/BitTable.h>
#include <immintrin.h>
#include <x86intrin.h>

constexpr size_t WIDTH = 4;

template <>
BitTable<uint64_t>::BitTable(const short order, const size_t cases_words,
                             const size_t ctrls_words)
    : order(order), size(std::pow(3, order)), cases_words(cases_words),
      ctrls_words(ctrls_words), alloc(std::make_unique<uint64_t[]>(
                                    size * (cases_words + ctrls_words) + 4)),
      cases((uint64_t *)((((uintptr_t)alloc.get()) + 31) / 32 * 32)),
      ctrls(cases + size * cases_words)
{
}

template <>
void BitTable<uint64_t>::fill(BitTable<uint64_t> &t, const uint64_t *cases1,
                              const uint64_t *ctrls1, const size_t size1,
                              const uint64_t *cases2,
                              const uint64_t *ctrls2) noexcept
{
    size_t i, j, k;
    // Compute bit tables for cases
    for (i = 0; i < size1; i += 3) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.cases_words; k += WIDTH) {
                __m256i y0 = _mm256_load_si256(
                    (__m256i *)(cases2 + j * t.cases_words + k));
                __m256i y1 = _mm256_load_si256(
                    (__m256i *)(cases1 + (i + 0) * t.cases_words + k));
                __m256i y2 = _mm256_load_si256(
                    (__m256i *)(cases1 + (i + 1) * t.cases_words + k));
                __m256i y3 = _mm256_load_si256(
                    (__m256i *)(cases1 + (i + 2) * t.cases_words + k));
                __m256i y4 = _mm256_and_si256(y0, y1);
                __m256i y5 = _mm256_and_si256(y0, y2);
                __m256i y6 = _mm256_and_si256(y0, y3);
                _mm256_store_si256(
                    (__m256i *)(t.cases + ((i + j) * 3 + 0) * t.cases_words +
                                k),
                    y4);
                _mm256_store_si256(
                    (__m256i *)(t.cases + ((i + j) * 3 + 1) * t.cases_words +
                                k),
                    y5);
                _mm256_store_si256(
                    (__m256i *)(t.cases + ((i + j) * 3 + 2) * t.cases_words +
                                k),
                    y6);
            }
        }
    }
    // Compute bit tables for ctrls
    for (i = 0; i < size1; i += 3) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.ctrls_words; k += WIDTH) {
                __m256i y0 = _mm256_load_si256(
                    (__m256i *)(ctrls2 + j * t.ctrls_words + k));
                __m256i y1 = _mm256_load_si256(
                    (__m256i *)(ctrls1 + (i + 0) * t.ctrls_words + k));
                __m256i y2 = _mm256_load_si256(
                    (__m256i *)(ctrls1 + (i + 1) * t.ctrls_words + k));
                __m256i y3 = _mm256_load_si256(
                    (__m256i *)(ctrls1 + (i + 2) * t.ctrls_words + k));
                __m256i y4 = _mm256_and_si256(y0, y1);
                __m256i y5 = _mm256_and_si256(y0, y2);
                __m256i y6 = _mm256_and_si256(y0, y3);
                _mm256_store_si256(
                    (__m256i *)(t.ctrls + ((i + j) * 3 + 0) * t.ctrls_words +
                                k),
                    y4);
                _mm256_store_si256(
                    (__m256i *)(t.ctrls + ((i + j) * 3 + 1) * t.ctrls_words +
                                k),
                    y5);
                _mm256_store_si256(
                    (__m256i *)(t.ctrls + ((i + j) * 3 + 2) * t.ctrls_words +
                                k),
                    y6);
            }
        }
    }
}

template <>
template <>
void BitTable<uint64_t>::popcnt(const uint64_t *cases1, const uint64_t *ctrls1,
                                const size_t size1, const uint64_t *cases2,
                                const uint64_t *ctrls2,
                                ContingencyTable<uint32_t> &t) noexcept
{
    size_t i, j, k;
    // Set tables to 0
    for (i = 0; i < t.size; i++) {
        t.cases[i] = 0;
        t.ctrls[i] = 0;
    }
    // Compute count tables for cases
    for (i = 0; i < size1; i += 3) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.cases_words; k += WIDTH) {
                __m256i y0 = _mm256_load_si256(
                    (__m256i *)(cases2 + j * t.cases_words + k));
                __m256i y1 = _mm256_load_si256(
                    (__m256i *)(cases1 + (i + 0) * t.cases_words + k));
                __m256i y2 = _mm256_load_si256(
                    (__m256i *)(cases1 + (i + 1) * t.cases_words + k));
                __m256i y3 = _mm256_load_si256(
                    (__m256i *)(cases1 + (i + 2) * t.cases_words + k));
                __m256i y4 = _mm256_and_si256(y0, y1);
                __m256i y5 = _mm256_and_si256(y0, y2);
                __m256i y6 = _mm256_and_si256(y0, y3);
                t.cases[(i + j) * 3 + 0] += _popcnt64(y4[0]) +
                                            _popcnt64(y4[1]) +
                                            _popcnt64(y4[2]) + _popcnt64(y4[3]);
                t.cases[(i + j) * 3 + 1] += _popcnt64(y5[0]) +
                                            _popcnt64(y5[1]) +
                                            _popcnt64(y5[2]) + _popcnt64(y5[3]);
                t.cases[(i + j) * 3 + 2] += _popcnt64(y6[0]) +
                                            _popcnt64(y6[1]) +
                                            _popcnt64(y6[2]) + _popcnt64(y6[3]);
            }
        }
    }
    // Compute count tables for ctrls
    for (i = 0; i < size1; i += 3) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.ctrls_words; k += WIDTH) {
                __m256i y0 = _mm256_load_si256(
                    (__m256i *)(ctrls2 + j * t.ctrls_words + k));
                __m256i y1 = _mm256_load_si256(
                    (__m256i *)(ctrls1 + (i + 0) * t.ctrls_words + k));
                __m256i y2 = _mm256_load_si256(
                    (__m256i *)(ctrls1 + (i + 1) * t.ctrls_words + k));
                __m256i y3 = _mm256_load_si256(
                    (__m256i *)(ctrls1 + (i + 2) * t.ctrls_words + k));
                __m256i y4 = _mm256_and_si256(y0, y1);
                __m256i y5 = _mm256_and_si256(y0, y2);
                __m256i y6 = _mm256_and_si256(y0, y3);
                t.ctrls[(i + j) * 3 + 0] += _popcnt64(y4[0]) +
                                            _popcnt64(y4[1]) +
                                            _popcnt64(y4[2]) + _popcnt64(y4[3]);
                t.ctrls[(i + j) * 3 + 1] += _popcnt64(y5[0]) +
                                            _popcnt64(y5[1]) +
                                            _popcnt64(y5[2]) + _popcnt64(y5[3]);
                t.ctrls[(i + j) * 3 + 2] += _popcnt64(y6[0]) +
                                            _popcnt64(y6[1]) +
                                            _popcnt64(y6[2]) + _popcnt64(y6[3]);
            }
        }
    }
}
