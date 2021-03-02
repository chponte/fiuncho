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
void BitTable<uint64_t>::combine(const BitTable<uint64_t> &t1,
                                 const BitTable<uint64_t> &t2,
                                 BitTable<uint64_t> &out) noexcept
{
    size_t i, j, k;
    // Compute bit tables for cases
    for (i = 0; i < t1.size; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t1.cases_words; k++) {
                out.cases[(i * 3 + j) * t1.cases_words + k] =
                    t1.cases[i * t1.cases_words + k] &
                    t2.cases[j * t1.cases_words + k];
            }
        }
    }
    // Compute bit tables for ctrls
    for (i = 0; i < t1.size; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t1.ctrls_words; k++) {
                out.ctrls[(i * 3 + j) * t1.ctrls_words + k] =
                    t1.ctrls[i * t1.ctrls_words + k] &
                    t2.ctrls[j * t1.ctrls_words + k];
            }
        }
    }
}

template <>
template <>
void BitTable<uint64_t>::combine_and_popcnt(
    const BitTable<uint64_t> &t1, const BitTable<uint64_t> &t2,
    ContingencyTable<uint32_t> &out) noexcept
{
    size_t i, j, k;
    // Set tables to 0
    for (i = 0; i < out.size; i++) {
        out.cases[i] = 0;
        out.ctrls[i] = 0;
    }
    // Compute count tables for cases
    for (i = 0; i < t1.size; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t1.cases_words; k++) {
                out.cases[i * 3 + j] +=
                    std::bitset<64>(t1.cases[i * t1.cases_words + k] &
                                    t2.cases[j * t1.cases_words + k])
                        .count();
            }
        }
    }
    // Compute count tables for ctrls
    for (i = 0; i < t1.size; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < t1.ctrls_words; k++) {
                out.ctrls[i * 3 + j] +=
                    std::bitset<64>(t1.ctrls[i * t1.ctrls_words + k] &
                                    t2.ctrls[j * t1.ctrls_words + k])
                        .count();
            }
        }
    }
}
