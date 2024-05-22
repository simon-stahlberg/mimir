/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_COMMON_RANDOM_HPP_
#define MIMIR_FORMALISM_COMMON_RANDOM_HPP_

#include <random>

class Random
{
public:
    static std::mt19937_64& get_rng()
    {
        static std::mt19937_64 rng(0);
        return rng;
    }

    // Utility function to generate a random uint64_t
    static uint64_t random_uint64()
    {
        std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
        return dist(get_rng());
    }
};

#endif