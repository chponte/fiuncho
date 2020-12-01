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

#include <gtest/gtest.h>
#include <bitset>
#include <fiuncho/engine/BitTable.h>
#include <fiuncho/engine/ContingencyTable.h>

#ifdef ALIGN
alignas(ALIGN)
#endif
    uint64_t cases1[3][8] = {
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff}};
#ifdef ALIGN
alignas(ALIGN)
#endif
    uint64_t cases2[3][8] = {
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff}};
#ifdef ALIGN
alignas(ALIGN)
#endif
    uint64_t ctrls1[3][16] = {
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555, 0xAAAAAAAAAAAAAAAA,
         0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x5555555555555555,
         0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000}};
#ifdef ALIGN
alignas(ALIGN)
#endif
    uint64_t ctrls2[3][16] = {
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555, 0xAAAAAAAAAAAAAAAA,
         0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x5555555555555555,
         0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000}};

namespace
{
TEST(BitTableTest, fill)
{
    uint64_t res_cases[9][8] = {
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff}};
    uint64_t res_ctrls[9][16] = {
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555, 0xAAAAAAAAAAAAAAAA,
         0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x5555555555555555,
         0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555, 0xAAAAAAAAAAAAAAAA,
         0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x5555555555555555,
         0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000, 0xAAAAAAAAAAAAAAAA,
         0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x5555555555555555,
         0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA,
         0x5555555555555555, 0x5555555555555555, 0xAAAAAAAAAAAAAAAA,
         0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x5555555555555555,
         0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000},
        {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
         0x5555555555555555, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000, 0xAAAAAAAAAAAAAAAA,
         0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x5555555555555555,
         0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000},
        {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000, 0x0000000000000000, 0xffffffffffffffff,
         0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
         0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
         0x0000000000000000}};

    BitTable<uint64_t> btable(2, 8, 16);

    EXPECT_EQ(9, btable.size);
    EXPECT_EQ(8, btable.cases_words);
    EXPECT_EQ(16, btable.ctrls_words);

    BitTable<uint64_t>::fill(btable, cases1[0], ctrls1[0], 3, cases2[0],
                             ctrls2[0]);

    for (auto i = 0; i < btable.size; i++) {
        for (auto j = 0; j < btable.cases_words; j++) {
            EXPECT_TRUE(btable.cases[i * btable.cases_words + j] ==
                        res_cases[i][j]);
        }
        for (auto j = 0; j < btable.ctrls_words; j++) {
            EXPECT_TRUE(btable.ctrls[i * btable.ctrls_words + j] ==
                        res_ctrls[i][j]);
        }
    }
}

TEST(BitTableTest, popcnt)
{
    uint32_t popcnt_cases[9] = {256, 128, 256, 128, 256, 256, 256, 256, 512};
    uint64_t popcnt_ctrls[9] = {512, 512, 256, 512, 1024, 512, 256, 512, 512};

    ContingencyTable<uint32_t> ctable(2, 8, 16);

#ifdef ALIGN
    EXPECT_EQ(16, ctable.size);
#else
    EXPECT_EQ(9, ctable.size);
#endif
    EXPECT_EQ(8, ctable.cases_words);
    EXPECT_EQ(16, ctable.ctrls_words);

    BitTable<uint64_t>::popcnt(cases1[0], ctrls1[0], 3, cases2[0], ctrls2[0],
                               ctable);

    for (auto i = 0; i < 9; i++) {
        EXPECT_EQ(ctable.cases[i], popcnt_cases[i]);
    }
#ifdef ALIGN
    for (auto i = 9; i < 16; i++) {
        EXPECT_EQ(ctable.cases[i], 0);
    }
#endif

    for (auto i = 0; i < 9; i++) {
        EXPECT_EQ(ctable.ctrls[i], popcnt_ctrls[i]);
    }
#ifdef ALIGN
    for (auto i = 9; i < 16; i++) {
        EXPECT_EQ(ctable.ctrls[i], 0);
    }
#endif
}
} // namespace
