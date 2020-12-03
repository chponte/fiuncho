#include <cmath>
#include <fiuncho/engine/BitTable.h>
#include <immintrin.h>
#include <x86intrin.h>

constexpr size_t WIDTH = 8;

template <>
BitTable<uint64_t>::BitTable(const short order, const size_t cases_words,
                             const size_t ctrls_words)
    : order(order), size(std::pow(3, order)), cases_words(cases_words),
      ctrls_words(ctrls_words), alloc(std::make_unique<uint64_t[]>(
                                    size * (cases_words + ctrls_words) + 8)),
      cases((uint64_t *)((((uintptr_t)alloc.get()) + 63) / 64 * 64)),
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
                __m512i z0 = _mm512_load_si512(
                    (__m512i *)(cases2 + j * t.cases_words + k));
                __m512i z1 = _mm512_load_si512(
                    (__m512i *)(cases1 + (i + 0) * t.cases_words + k));
                __m512i z2 = _mm512_load_si512(
                    (__m512i *)(cases1 + (i + 1) * t.cases_words + k));
                __m512i z3 = _mm512_load_si512(
                    (__m512i *)(cases1 + (i + 2) * t.cases_words + k));
                __m512i z4 = _mm512_and_si512(z0, z1);
                __m512i z5 = _mm512_and_si512(z0, z2);
                __m512i z6 = _mm512_and_si512(z0, z3);
                _mm512_store_si512(
                    (__m512i *)(t.cases + ((i + j) * 3 + 0) * t.cases_words +
                                k),
                    z4);
                _mm512_store_si512(
                    (__m512i *)(t.cases + ((i + j) * 3 + 1) * t.cases_words +
                                k),
                    z5);
                _mm512_store_si512(
                    (__m512i *)(t.cases + ((i + j) * 3 + 2) * t.cases_words +
                                k),
                    z6);
            }
        }
    }
    // Compute bit tables for ctrls
    for (i = 0; i < size1; i += 3) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.ctrls_words; k += WIDTH) {
                __m512i z0 = _mm512_load_si512(
                    (__m512i *)(ctrls2 + j * t.ctrls_words + k));
                __m512i z1 = _mm512_load_si512(
                    (__m512i *)(ctrls1 + (i + 0) * t.ctrls_words + k));
                __m512i z2 = _mm512_load_si512(
                    (__m512i *)(ctrls1 + (i + 1) * t.ctrls_words + k));
                __m512i z3 = _mm512_load_si512(
                    (__m512i *)(ctrls1 + (i + 2) * t.ctrls_words + k));
                __m512i z4 = _mm512_and_si512(z0, z1);
                __m512i z5 = _mm512_and_si512(z0, z2);
                __m512i z6 = _mm512_and_si512(z0, z3);
                _mm512_store_si512(
                    (__m512i *)(t.ctrls + ((i + j) * 3 + 0) * t.ctrls_words +
                                k),
                    z4);
                _mm512_store_si512(
                    (__m512i *)(t.ctrls + ((i + j) * 3 + 1) * t.ctrls_words +
                                k),
                    z5);
                _mm512_store_si512(
                    (__m512i *)(t.ctrls + ((i + j) * 3 + 2) * t.ctrls_words +
                                k),
                    z6);
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
                __m512i z0 = _mm512_load_si512(
                    (__m512i *)(cases2 + j * t.cases_words + k));
                __m512i z1 = _mm512_load_si512(
                    (__m512i *)(cases1 + (i + 0) * t.cases_words + k));
                __m512i z2 = _mm512_load_si512(
                    (__m512i *)(cases1 + (i + 1) * t.cases_words + k));
                __m512i z3 = _mm512_load_si512(
                    (__m512i *)(cases1 + (i + 2) * t.cases_words + k));
                __m512i z4 = _mm512_and_si512(z0, z1);
                __m512i z5 = _mm512_and_si512(z0, z2);
                __m512i z6 = _mm512_and_si512(z0, z3);
                t.cases[(i + j) * 3 + 0] +=
                    _popcnt64(z4[0]) + _popcnt64(z4[1]) + _popcnt64(z4[2]) +
                    _popcnt64(z4[3]) + _popcnt64(z4[4]) + _popcnt64(z4[5]) +
                    _popcnt64(z4[6]) + _popcnt64(z4[7]);
                t.cases[(i + j) * 3 + 1] +=
                    _popcnt64(z5[0]) + _popcnt64(z5[1]) + _popcnt64(z5[2]) +
                    _popcnt64(z5[3]) + _popcnt64(z5[4]) + _popcnt64(z5[5]) +
                    _popcnt64(z5[6]) + _popcnt64(z5[7]);
                t.cases[(i + j) * 3 + 2] +=
                    _popcnt64(z6[0]) + _popcnt64(z6[1]) + _popcnt64(z6[2]) +
                    _popcnt64(z6[3]) + _popcnt64(z6[4]) + _popcnt64(z6[5]) +
                    _popcnt64(z6[6]) + _popcnt64(z6[7]);
            }
        }
    }
    // Compute count tables for ctrls
    for (i = 0; i < size1; i += 3) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.ctrls_words; k += WIDTH) {
                __m512i z0 = _mm512_load_si512(
                    (__m512i *)(ctrls2 + j * t.ctrls_words + k));
                __m512i z1 = _mm512_load_si512(
                    (__m512i *)(ctrls1 + (i + 0) * t.ctrls_words + k));
                __m512i z2 = _mm512_load_si512(
                    (__m512i *)(ctrls1 + (i + 1) * t.ctrls_words + k));
                __m512i z3 = _mm512_load_si512(
                    (__m512i *)(ctrls1 + (i + 2) * t.ctrls_words + k));
                __m512i z4 = _mm512_and_si512(z0, z1);
                __m512i z5 = _mm512_and_si512(z0, z2);
                __m512i z6 = _mm512_and_si512(z0, z3);
                t.ctrls[(i + j) * 3 + 0] +=
                    _popcnt64(z4[0]) + _popcnt64(z4[1]) + _popcnt64(z4[2]) +
                    _popcnt64(z4[3]) + _popcnt64(z4[4]) + _popcnt64(z4[5]) +
                    _popcnt64(z4[6]) + _popcnt64(z4[7]);
                t.ctrls[(i + j) * 3 + 1] +=
                    _popcnt64(z5[0]) + _popcnt64(z5[1]) + _popcnt64(z5[2]) +
                    _popcnt64(z5[3]) + _popcnt64(z5[4]) + _popcnt64(z5[5]) +
                    _popcnt64(z5[6]) + _popcnt64(z5[7]);
                t.ctrls[(i + j) * 3 + 2] +=
                    _popcnt64(z6[0]) + _popcnt64(z6[1]) + _popcnt64(z6[2]) +
                    _popcnt64(z6[3]) + _popcnt64(z6[4]) + _popcnt64(z6[5]) +
                    _popcnt64(z6[6]) + _popcnt64(z6[7]);
            }
        }
    }
}
