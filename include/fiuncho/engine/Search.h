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

#include <fiuncho/dataset/Dataset.h>
#include <fiuncho/engine/Distributor.h>
#include <fiuncho/engine/Result.h>

#ifndef FIUNCHO_SEARCH_H
#define FIUNCHO_SEARCH_H

class Search
{
  public:
    virtual std::vector<Result<uint32_t, float>>
    run(const Dataset<uint64_t> &dataset, const unsigned short order,
        Distributor<uint32_t> distributor, const unsigned int outputs) = 0;
};

#endif
