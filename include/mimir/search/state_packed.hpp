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

    template<formalism::IsFluentOrDerivedTag P>
    auto get_atoms(const formalism::ProblemImpl& problem) const;
    const FlatDoubleList& get_numeric_variables(const formalism::ProblemImpl& problem) const;

    /**
     * Utils
     */

    /// @brief Check whether the literal holds in the state using binary search over the atoms in the state.
    /// @tparam P is the literal type.
    /// @param literal is the literal.
    /// @return true if the literal holds in the state, and false otherwise.
    template<formalism::IsFluentOrDerivedTag P>
    bool literal_holds(formalism::GroundLiteral<P> literal, const formalism::ProblemImpl& problem) const;

    /// @brief Check whether all literals hold in the state using binary searches over the atoms in the state.
    /// @tparam P is the literal type.
    /// @param literals are the literals.
    /// @return true if all literals hold in the state, and false otherwise.
    template<formalism::IsFluentOrDerivedTag P>
    bool literals_hold(const formalism::GroundLiteralList<P>& literals, const formalism::ProblemImpl& problem) const;

    template<formalism::IsFluentOrDerivedTag P, std::ranges::input_range Range1, std::ranges::input_range Range2>
        requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
    bool literals_hold(const Range1& positive_atoms, const Range2& negative_atoms, const formalism::ProblemImpl& problem) const;
};

static_assert(sizeof(PackedStateImpl) == 20);

/**
 * Implementations
 */

template<formalism::IsFluentOrDerivedTag P>
auto PackedStateImpl::get_atoms(const formalism::ProblemImpl& problem) const
{
    return std::ranges::subrange(valla::plain::begin(get_atoms<P>(), problem.get_tree_table()), valla::plain::end());
}

template<formalism::IsFluentOrDerivedTag P, std::ranges::input_range Range1, std::ranges::input_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
bool PackedStateImpl::literals_hold(const Range1& positive_atoms, const Range2& negative_atoms, const formalism::ProblemImpl& problem) const
{
    auto atoms = get_atoms<P>(problem);
    return is_supseteq(atoms, positive_atoms) && are_disjoint(atoms, negative_atoms);
}

}

namespace loki
{
/// @private
template<>
struct Hash<mimir::search::PackedStateImpl>
{
    size_t operator()(const mimir::search::PackedStateImpl& el) const;
};
/// @private
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
