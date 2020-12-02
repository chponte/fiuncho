#include <bitset>
#include <cmath>
#include <fiuncho/engine/BitTable.h>

template <>
BitTable<uint64_t>::BitTable(const short order, const size_t cases_words,
                             const size_t ctrls_words)
    : order(order), size(std::pow(3, order)), cases_words(cases_words),
      ctrls_words(ctrls_words),
      alloc(std::make_unique<uint64_t[]>(size * (cases_words + ctrls_words))),
      cases(alloc.get()), ctrls(cases + size * cases_words)
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
    for (i = 0; i < size1; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.cases_words; k++) {
                t.cases[(i * 3 + j) * t.cases_words + k] =
                    cases1[i * t.cases_words + k] &
                    cases2[j * t.cases_words + k];
            }
        }
    }
    // Compute bit tables for ctrls
    for (i = 0; i < size1; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.ctrls_words; k++) {
                t.ctrls[(i * 3 + j) * t.ctrls_words + k] =
                    ctrls1[i * t.ctrls_words + k] &
                    ctrls2[j * t.ctrls_words + k];
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
    for (i = 0; i < size1; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.cases_words; k++) {
                t.cases[i * 3 + j] +=
                    std::bitset<64>(cases1[i * t.cases_words + k] &
                                    cases2[j * t.cases_words + k])
                        .count();
            }
        }
    }
    // Compute count tables for ctrls
    for (i = 0; i < size1; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t.ctrls_words; k++) {
                t.ctrls[i * 3 + j] +=
                    std::bitset<64>(ctrls1[i * t.ctrls_words + k] &
                                    ctrls2[j * t.ctrls_words + k])
                        .count();
            }
        }
    }
}
