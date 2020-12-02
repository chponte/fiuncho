#ifndef FIUNCHO_BITTABLE_H
#define FIUNCHO_BITTABLE_H

#include <fiuncho/engine/ContingencyTable.h>

#include <cstddef>
#include <cstdint>
#include <memory>

template <class T> class BitTable
{
  public:
    BitTable(const short order, const size_t cases_words,
             const size_t ctrls_words);

    static void fill(BitTable<T> &t, const uint64_t *cases1,
                     const uint64_t *ctrls1, const size_t size1,
                     const uint64_t *cases2, const uint64_t *ctrls2) noexcept;

    template <class U>
    static void popcnt(const uint64_t *cases1, const uint64_t *ctrls1,
                       const size_t size1, const uint64_t *cases2,
                       const uint64_t *ctrls2, ContingencyTable<U> &t) noexcept;

    const size_t order, size, cases_words, ctrls_words;

  private:  
    std::unique_ptr<T[]> alloc;

  public:
    T *cases, *ctrls;
};

#endif