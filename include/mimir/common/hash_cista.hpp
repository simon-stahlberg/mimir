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

#ifndef MIMIR_COMMON_HASH_CISTA_HPP_
#define MIMIR_COMMON_HASH_CISTA_HPP_

#include "cista/containers/dynamic_bitset.h"
#include "cista/containers/tuple.h"
#include "cista/containers/vector.h"
#include "mimir/common/hash.hpp"

/* DynamicBitset */

template<typename Block, template<typename> typename Ptr>
struct std::hash<cista::basic_dynamic_bitset<Block, Ptr>>
{
    using Type = cista::basic_dynamic_bitset<Block, Ptr>;

    size_t operator()(const Type& bitset) const
    {
        const auto default_block = bitset.default_bit_value_ ? Type::block_ones : Type::block_zeroes;
        size_t seed = default_block;

        // Find the last block that differs from the default block
        auto last_relevant_index = static_cast<int64_t>(bitset.blocks_.size()) - 1;
        for (; (last_relevant_index >= 0) && (bitset.blocks_[last_relevant_index] == default_block); --last_relevant_index) {}
        size_t hashable_size = last_relevant_index + 1;

        // Compute a hash value up to and including this block
        for (size_t i = 0; i < hashable_size; ++i)
        {
            mimir::hash_combine(seed, bitset.blocks_[i]);
        }

        return seed;
    }
};

/* Tuple */

template<typename... Ts>
struct std::hash<cista::tuple<Ts...>>
{
    using Type = cista::tuple<Ts...>;

    size_t operator()(const Type& tuple) const
    {
        constexpr std::size_t seed = sizeof...(Ts);

        [&]<std::size_t... Is>(std::index_sequence<Is...>) { (mimir::hash_combine(seed, cista::get<Is>(tuple)), ...); }
        (std::make_index_sequence<sizeof...(Ts)> {});

        return seed;
    }
};

/* Vector */

template<typename T, template<typename> typename Ptr, bool IndexPointers, typename TemplateSizeType, class Allocator>
struct std::hash<cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>>
{
    using Type = cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>;

    size_t operator()(const Type& vector) const
    {
        size_t seed = vector.size();
        for (const auto& element : vector)
        {
            mimir::hash_combine(seed, element);
        }
        return seed;
    }
};

#endif
