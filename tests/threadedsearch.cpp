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

#include "utils.h"
#include <fiuncho/Distributor.h>
#include <fiuncho/Search.h>
#include <fiuncho/ThreadedSearch.h>
#include <fiuncho/dataset/Dataset.h>
#include <gtest/gtest.h>

std::string tped, tfam;

namespace
{
TEST(ThreadedSearchTest, SingleThread)
{
    // Run ThreadedSearch
#ifdef ALIGN
    const Dataset<uint64_t> dataset =
        Dataset<uint64_t>::read<ALIGN>(tped, tfam);
#else
    const Dataset<uint64_t> dataset = Dataset<uint64_t>::read(tped, tfam);
#endif
    ThreadedSearch search(1);
    Distributor<uint32_t> distributor(dataset.snps, 1, 0);

    for (auto o = 2; o < 5; o++) {
        auto result = search.run(dataset, o, distributor, 100);
        EXPECT_FALSE(has_repeated_elements(result));
        EXPECT_TRUE(ascending_combinations(result));
        if (o == 3) {
            EXPECT_TRUE(matches_mpi3snp_output(result));
        }
    }
}

TEST(ThreadedSearchTest, MultiThread)
{
    // Run ThreadedSearch
#ifdef ALIGN
    const Dataset<uint64_t> dataset =
        Dataset<uint64_t>::read<ALIGN>(tped, tfam);
#else
    const Dataset<uint64_t> dataset = Dataset<uint64_t>::read(tped, tfam);
#endif
    ThreadedSearch search(32);
    Distributor<uint32_t> distributor(dataset.snps, 1, 0);

    for (auto o = 2; o < 5; o++) {
        auto result = search.run(dataset, o, distributor, 100);
        EXPECT_FALSE(has_repeated_elements(result));
        EXPECT_TRUE(ascending_combinations(result));
        if (o == 3) {
            EXPECT_TRUE(matches_mpi3snp_output(result));
        }
    }
}
} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    assert(argc == 3); // gtest leaved unparsed arguments for you
    tped = argv[1];
    tfam = argv[2];
    return RUN_ALL_TESTS();
}