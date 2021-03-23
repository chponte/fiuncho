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
 * @file ContingencyTable.h
 * @author Christian Ponte
 * @date 29 May 2018
 *
 * @brief ContingencyTable class declaration.
 */

#ifndef FIUNCHO_CONTINGENCYTABLE_H
#define FIUNCHO_CONTINGENCYTABLE_H

#include <cstddef>
#include <cstdint>
#include <memory>

template <class U> class ContingencyTable
{
  public:
    ContingencyTable(const short order, const size_t cases_words,
                     const size_t ctrls_words);

    const size_t size, cases_words, ctrls_words;

  private:
    std::unique_ptr<U[]> alloc;

  public:
    U *cases, *ctrls;
};

#endif
