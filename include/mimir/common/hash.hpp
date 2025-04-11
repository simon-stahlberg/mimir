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

#ifndef MIMIR_COMMON_HASH_HPP_
#define MIMIR_COMMON_HASH_HPP_

#include "cista/containers/dual_dynamic_bitset.h"
#include "cista/containers/dynamic_bitset.h"
#include "cista/containers/external_ptr.h"
#include "cista/containers/flexible_index_vector.h"
#include "cista/containers/tuple.h"
#include "cista/containers/vector.h"

#include <loki/details/utils/hash.hpp>

/**
 * We inject additional hash specializations into the loki::Hash.
 */

/* ExternalPtr */

template<typename T>
struct loki::Hash<cista::basic_external_ptr<T>>
{
    using Type = cista::basic_external_ptr<T>;

    size_t operator()(const Type& ptr) const { return loki::hash_combine(ptr.el_); }
};

/* DualDynamicBitset */

template<typename Block, template<typename> typename Ptr>
struct loki::Hash<cista::basic_dual_dynamic_bitset<Block, Ptr>>
{
    using Type = cista::basic_dual_dynamic_bitset<Block, Ptr>;

    size_t operator()(const Type& bitset) const
    {
        const auto default_block = bitset.default_bit_value_ ? Type::block_ones : Type::block_zeroes;

        // Find the last block that differs from the default block
        auto last_relevant_index = static_cast<int64_t>(bitset.blocks_.size()) - 1;
        for (; (last_relevant_index >= 0) && (bitset.blocks_[last_relevant_index] == default_block); --last_relevant_index) {}
        size_t hashable_size = last_relevant_index + 1;

        // Compute a hash value up to and including this block
        size_t seed = loki::hash_combine(default_block, hashable_size);
        size_t hash[2] = { 0, 0 };

        loki::MurmurHash3_x64_128(bitset.blocks_.data(), hashable_size * sizeof(Block), seed, hash);

        loki::hash_combine(seed, hash[0]);
        loki::hash_combine(seed, hash[1]);

        return seed;
    }
};

/* DynamicBitset */

template<typename Block, template<typename> typename Ptr>
struct loki::Hash<cista::basic_dynamic_bitset<Block, Ptr>>
{
    using Type = cista::basic_dynamic_bitset<Block, Ptr>;

    size_t operator()(const Type& bitset) const
    {
        // Find the last block that differs from the default block
        auto last_relevant_index = static_cast<int64_t>(bitset.blocks_.size()) - 1;
        for (; (last_relevant_index >= 0) && (bitset.blocks_[last_relevant_index] == Type::block_zeroes); --last_relevant_index) {}
        size_t hashable_size = last_relevant_index + 1;

        // Compute a hash value up to and including this block
        size_t seed = Type::block_zeroes;
        loki::hash_combine(seed, hashable_size);
        size_t hash[2] = { 0, 0 };

        loki::MurmurHash3_x64_128(bitset.blocks_.data(), hashable_size * sizeof(Block), seed, hash);

        loki::hash_combine(seed, hash[0]);
        loki::hash_combine(seed, hash[1]);

        return seed;
    }
};

/* Tuple */

template<typename... Ts>
struct loki::Hash<cista::tuple<Ts...>>
{
    using Type = cista::tuple<Ts...>;

    size_t operator()(const Type& tuple) const
    {
        constexpr std::size_t aggregated_hash = sizeof...(Ts);

        [&]<std::size_t... Is>(std::index_sequence<Is...>)
        { (loki::hash_combine(aggregated_hash, cista::get<Is>(tuple)), ...); }(std::make_index_sequence<sizeof...(Ts)> {});

        return aggregated_hash;
    }
};

/* Vector */

template<typename T, template<typename> typename Ptr, bool IndexPointers, typename TemplateSizeType, class Allocator>
struct loki::Hash<cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>>
{
    using Type = cista::basic_vector<T, Ptr, IndexPointers, TemplateSizeType, Allocator>;

    size_t operator()(const Type& vector) const
    {
        size_t aggregated_hash = vector.size();

        for (const auto& element : vector)
        {
            loki::hash_combine(aggregated_hash, loki::Hash<T> {}(element));
        }

        return aggregated_hash;
    }
};

/* FlexibleIndexVector */

template<std::unsigned_integral IndexType, template<typename> typename Ptr>
struct loki::Hash<cista::basic_flexible_index_vector<IndexType, Ptr>>
{
    using Type = cista::basic_flexible_index_vector<IndexType, Ptr>;

    size_t operator()(const Type& vector) const
    {
        size_t seed = vector.size();
        size_t hash[2] = { 0, 0 };

        loki::MurmurHash3_x64_128(vector.blocks().data(), vector.blocks().size() * sizeof(IndexType), seed, hash);

        loki::hash_combine(seed, hash[0]);
        loki::hash_combine(seed, hash[1]);

        return seed;
    }
};

#endif
