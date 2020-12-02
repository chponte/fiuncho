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
 * @date 29 May 2018
 *
 * @brief Dataset class declaration, responsible for reading the data and
 * storing it using a bitwise representation.
 */

#ifndef FIUNCHO_DATASET_H
#define FIUNCHO_DATASET_H

#include <array>
#include <fiuncho/dataset/Individual.h>
#include <fiuncho/dataset/SNP.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

template <class T> class Dataset
{
  public:
    static Dataset read(std::string tped, std::string tfam)
    {
        std::vector<Individual> individuals;
        std::vector<SNP> snps;
        size_t cases_count, ctrls_count;
        read_individuals(tfam, individuals, cases_count, ctrls_count);
        read_snps(tped, individuals, snps);
        // Allocate enough space for representing all SNPs for all individuals
        constexpr size_t NBITS = sizeof(T) * 8; // Number of bits in T
        const size_t cases_words = (cases_count + NBITS - 1) / NBITS,
                     ctrls_words = (ctrls_count + NBITS - 1) / NBITS;
        // Find the address of the first aligned position inside the allocation
        T *alloc =
            (T *)new T[(cases_words + ctrls_words) * 3 * snps.size()];

        std::vector<std::array<const T *, 3>> cases, ctrls;
        populate(individuals, snps, alloc, cases_words, cases,
                 alloc + cases_words * 3 * snps.size(), ctrls_words, ctrls);

        return std::move(Dataset(alloc, cases, cases_words, cases_count, ctrls,
                         ctrls_words, ctrls_count));
    }

    const size_t cases_words, cases_count, ctrls_words, ctrls_count, inds_count;
    const std::vector<std::array<const T *, 3>> cases, ctrls;

  private:
    Dataset(T *alloc, std::vector<std::array<const T *, 3>> cases,
            size_t cases_words, size_t cases_count,
            std::vector<std::array<const T *, 3>> ctrls, size_t ctrls_words,
            size_t ctrls_count)
        : alloc(alloc), cases(cases), cases_words(cases_words),
          cases_count(cases_count), ctrls(ctrls), ctrls_words(ctrls_words),
          ctrls_count(ctrls_count), inds_count(cases_count + ctrls_count)
    {
    }

    inline static void read_individuals(const std::string &tfam,
                                        std::vector<Individual> &individuals,
                                        size_t &cases, size_t &ctrls)
    {
        std::ifstream file;
        file.open(tfam.c_str(), std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Error while opening " + tfam +
                                     ", check file path/permissions");
        }
        try {
            Individual ind;
            ctrls = 0;
            while (file >> ind) {
                individuals.push_back(ind);
                ctrls += ind.ph == 1;
            }
            cases = individuals.size() - ctrls;
        } catch (const Individual::InvalidIndividual &e) {
            throw std::runtime_error("Error in " + tfam + ":" +
                                     std::to_string(individuals.size() + 1) +
                                     ": " + e.what());
        }
        file.close();
    }

    inline static void read_snps(const std::string &tped,
                                 const std::vector<Individual> &individuals,
                                 std::vector<SNP> &snps)
    {
        std::ifstream file;
        file.open(tped.c_str(), std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Error while opening " + tped +
                                     ", check file path/permissions");
        }
        try {
            SNP snp;
            while (file >> snp) {
                if (snp.genotypes.size() == individuals.size()) {
                    snps.push_back(snp);
                } else {
                    throw std::runtime_error(
                        "Error in " + tped + ":" +
                        std::to_string(snps.size() + 1) +
                        ": the number of nucleotides does not match "
                        "the number of individuals");
                }
            }
        } catch (const SNP::InvalidSNP &e) {
            throw std::runtime_error("Error in " + tped + ":" +
                                     std::to_string(snps.size() + 1) + ": " +
                                     e.what());
        }
        file.close();
    }

    inline static void populate(const std::vector<Individual> &inds,
                                const std::vector<SNP> &snps, T *cases_ptr,
                                const size_t cases_words,
                                std::vector<std::array<const T *, 3>> &cases,
                                T *ctrls_ptr, const size_t ctrls_words,
                                std::vector<std::array<const T *, 3>> &ctrls)
    {
        constexpr size_t BITS = sizeof(T) * 8; // Number of bits in T
        // Buffers
        T *cases_buff[3], *ctrls_buff[3];
        size_t cases_cnt, ctrls_cnt;
        for (auto i = 0; i < snps.size(); i++) {
            // Initialize buffers
            cases_cnt = 0;
            ctrls_cnt = 0;
            for (auto j = 0; j < 3; j++) {
                cases_buff[j] = cases_ptr + cases_words * j;
                for (auto k = 0; k < cases_words; k++) {
                    cases_buff[j][k] = 0;
                }
            }
            cases_ptr += 3 * cases_words;
            for (auto j = 0; j < 3; j++) {
                ctrls_buff[j] = ctrls_ptr + ctrls_words * j;
                for (auto k = 0; k < ctrls_words; k++) {
                    ctrls_buff[j][k] = 0;
                }
            }
            ctrls_ptr += 3 * ctrls_words;
            for (auto j = 0; j < inds.size(); j++) {
                if (inds[j].ph == 1) {
                    for (auto k = 0; k < 3; k++) {
                        ctrls_buff[k][ctrls_cnt / BITS] =
                            (ctrls_buff[k][ctrls_cnt / BITS] << 1) +
                            (snps[i].genotypes[j] == k);
                    }
                    ctrls_cnt++;
                } else {
                    for (auto k = 0; k < 3; k++) {
                        cases_buff[k][cases_cnt / BITS] =
                            (cases_buff[k][cases_cnt / BITS] << 1) +
                            (snps[i].genotypes[j] == k);
                    }
                    cases_cnt++;
                }
            }
            cases.emplace_back(std::array<const T *, 3>{
                cases_buff[0], cases_buff[1], cases_buff[2]});
            ctrls.emplace_back(std::array<const T *, 3>{
                ctrls_buff[0], ctrls_buff[1], ctrls_buff[2]});
        }
    }

    std::unique_ptr<T[]> alloc;
};

#endif
