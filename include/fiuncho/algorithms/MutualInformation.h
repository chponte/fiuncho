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
 * @file MutualInformation.h
 * @author Christian Ponte
 * @date 30 May 2018
 *
 * @brief MutualInformation class declaration.
 */

#ifndef FIUNCHO_MUTUALINFORMATION_H
#define FIUNCHO_MUTUALINFORMATION_H

#include <fiuncho/algorithms/Algorithm.h>

template <class T>
class MutualInformation : public Algorithm<T> {
  public:
    MutualInformation(unsigned int num_cases, unsigned int num_ctrls);

    template <class U>
    T compute(const ContingencyTable<U> &table) const noexcept;

  private:
    T inv_inds;
    // Entropy of Y
    T h_y;
};

#endif
