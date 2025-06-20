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

/// @brief `PackedStateImpl` encapsulates the fluent and derived atoms, and numeric variables of a planning state in a compressed way.
class PackedStateImpl
{
private:
    Index m_fluent_atoms_index;
    Index m_fluent_atoms_size;
    Index m_derived_atoms_index;
    Index m_derived_atoms_size;
    Index m_numeric_variables;

    PackedStateImpl(v::RootSlotType fluent_atoms, v::RootSlotType derived_atoms, Index numeric_variables);

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
    Index get_numeric_variables() const;
};

static_assert(sizeof(PackedStateImpl) == 20);

}

namespace loki
{
template<>
struct Hash<mimir::search::PackedStateImpl>
{
    size_t operator()(const mimir::search::PackedStateImpl& el) const;
};

template<>
struct EqualTo<mimir::search::PackedStateImpl>
{
    bool operator()(const mimir::search::PackedStateImpl& lhs, const mimir::search::PackedStateImpl& rhs) const;
};

}

namespace mimir::search
{
using PackedStateImplMap = absl::node_hash_map<PackedStateImpl, Index, loki::Hash<PackedStateImpl>, loki::EqualTo<PackedStateImpl>>;
}

#endif
