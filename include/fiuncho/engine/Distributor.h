/*
 * This file is part of Fiuncho.
 *
 * Fiuncho is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fiuncho is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fiuncho. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file Dataset.h
 * @author Christian Ponte
 * @date 1 March 2018
 *
 * @brief Distributor class declaration. This class implements the parallel
 * distribution strategy, shared by both cpu and gpu implementation.
 */

#ifndef FIUNCHO_DISTRIBUTOR_H
#define FIUNCHO_DISTRIBUTOR_H

#include <vector>

template <typename T> class PairList
{
    std::pair<T, T> p;
    T i, j, step;

  public:
    PairList(T i, T j, T offset, T step)
        : p(std::make_pair<T, T>(offset / j, offset % j)), i(i), j(j),
          step(step)
    {
    }

    class iterator
    {
        PairList<T> *list;

      public:
        using value_type = T;
        using reference = T;
        using iterator_category = std::input_iterator_tag;
        using pointer = T *;
        using difference_type = void;

        iterator(PairList *list) : list(list) {}

        std::pair<T, T> operator*() const { return list->p; }
        std::pair<T, T> *operator->() const { return &list->p; }

        iterator &operator++()
        { // preincrement
            list->p.second += list->step;
            while (list->p.first < list->i) {
                while (list->p.second < list->j) {
                    return *this;
                }
                list->p.first++;
                list->p.second = list->p.first + 1 + (list->p.second % list->j);
            }
            this->list = nullptr;
            return *this;
        }

        friend bool operator==(iterator const &lhs, iterator const &rhs)
        {
            return lhs.list == rhs.list;
        }
        friend bool operator!=(iterator const &lhs, iterator const &rhs)
        {
            return !(lhs == rhs);
        }
    };

    iterator begin() { return iterator(this); }
    iterator end() { return iterator(nullptr); }
};

template <typename T> class Distributor
{
  public:
    Distributor(const T &size, const unsigned int &frac, const unsigned int &id)
        : size(size), frac(frac), id(id){};

    Distributor<T> layer(const unsigned int &frac, const unsigned int &id) const
    {
        return Distributor<T>(this->size, this->frac * frac,
                              this->id * frac + id);
    }

    PairList<T> get_pairs() const noexcept {
        return PairList<T>(size, size, id, frac);
    }

  private:
    const T size;
    const unsigned int frac, id;
};

#endif
