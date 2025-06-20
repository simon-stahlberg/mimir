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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_SEARCH_STATE_PACKED_HPP_
#define MIMIR_SEARCH_STATE_PACKED_HPP_

#include "mimir/common/hash.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/state_unpacked.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <memory>
#include <valla/indexed_hash_set.hpp>
#include <valla/tree_compression.hpp>

namespace mimir::search
{
namespace v = valla::plain;

/**
 * PackedState
 */

/// @brief `PackedStateImpl` encapsulates the fluent and derived atoms, and numeric variables of a planning state in a compressed way.
class PackedStateImpl
{
private:
    Index m_fluent_atoms_index;
    Index m_fluent_atoms_size;
    Index m_derived_atoms_index;
    Index m_derived_atoms_size;
    Index m_numeric_variables_index;
    Index m_numeric_variables_size;

    PackedStateImpl(v::RootSlotType fluent_atoms, v::RootSlotType derived_atoms, v::RootSlotType numeric_variables);

    friend class StateRepositoryImpl;

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    /**
     * Getters
     */

    template<formalism::IsFluentOrDerivedTag P>
    v::RootSlotType get_atoms() const;
    v::RootSlotType get_numeric_variables() const;
};

static_assert(sizeof(PackedStateImpl) == 24);

}

namespace loki
{
template<>
struct Hash<mimir::search::PackedStateImpl>
{
    size_t operator()(const mimir::search::PackedStateImpl& el) const
    {
        static_assert(std::is_standard_layout_v<mimir::search::PackedStateImpl>, "PackedStateImpl must be standard layout");

        size_t seed = 0;
        size_t hash[2] = { 0, 0 };

        loki::MurmurHash3_x64_128(reinterpret_cast<const uint8_t*>(&el), sizeof(mimir::search::PackedStateImpl), seed, hash);

        loki::hash_combine(seed, hash[0]);
        loki::hash_combine(seed, hash[1]);

        return seed;
    }
};

template<>
struct EqualTo<mimir::search::PackedStateImpl>
{
    bool operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const
    {
        static_assert(std::is_standard_layout_v<mimir::search::PackedStateImpl>, "PackedStateImpl must be standard layout");

        const auto lhs_begin = reinterpret_cast<const uint8_t*>(&lhs);
        const auto rhs_begin = reinterpret_cast<const uint8_t*>(&rhs);

        return std::equal(lhs_begin, lhs_begin + sizeof(mimir::search::PackedStateImpl), rhs_begin);
    }
};

}

namespace mimir::search
{
using PackedStateImplMap = absl::node_hash_map<PackedStateImpl, Index, loki::Hash<PackedStateImpl>, loki::EqualTo<PackedStateImpl>>;
}

#endif
