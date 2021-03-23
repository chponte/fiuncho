#ifndef FIUNCHO_GENOTYPETABLE_H
#define FIUNCHO_GENOTYPETABLE_H

#include <fiuncho/ContingencyTable.h>

#include <cstddef>
#include <cstdint>
#include <memory>

template <class T> class GenotypeTable
{
  public:
    GenotypeTable(T *cases, const size_t cases_words, T *ctrls,
                  const size_t ctrls_words)
        : order(1), size(3), cases(cases), cases_words(cases_words),
          ctrls(ctrls), ctrls_words(ctrls_words), alloc(nullptr){};

    GenotypeTable(const short order, const size_t cases_words,
                  const size_t ctrls_words);

    static void combine(const GenotypeTable<T> &t1, const GenotypeTable<T> &t2,
                        GenotypeTable<T> &out) noexcept;

    template <class U>
    static void combine_and_popcnt(const GenotypeTable<T> &t1,
                                   const GenotypeTable<T> &t2,
                                   ContingencyTable<U> &out) noexcept;

    const size_t order, size, cases_words, ctrls_words;

  private:
    std::unique_ptr<T[]> alloc;

  public:
    T *cases, *ctrls;
};

#endif