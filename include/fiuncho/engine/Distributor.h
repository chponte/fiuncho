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

template <typename T> class Distributor {
  public:
    Distributor(const T &size, const unsigned int &frac, const unsigned int &id)
        : size(size), frac(frac), id(id){};

    Distributor<T> layer(const unsigned int &frac,
                         const unsigned int &id) const {
        return Distributor<T>(this->size, this->frac * frac,
                              this->id * frac + id);
    }

    void get_pairs(std::vector<std::pair<T, T>> &pairs) const noexcept {
        const size_t total_pairs = size * (size - 1) / 2;
        T i, j, offset;
        offset = id;
        for (i = 0; i < size; i++) {
            for (j = i + 1 + offset; j < size; j += frac) {
                pairs.emplace_back(i, j);
            }
            offset = j - size;
        }
    }

  private:
    const T size;
    const unsigned int frac, id;
};

#endif
