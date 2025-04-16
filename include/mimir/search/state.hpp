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

#ifndef MIMIR_SEARCH_STATE_HPP_
#define MIMIR_SEARCH_STATE_HPP_

#include "mimir/buffering/unordered_set.h"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <valla/delta_tree_compression.hpp>
#include <valla/indexed_hash_set.hpp>
#include <valla/tree_compression.hpp>

namespace mimir::search
{
namespace v = valla::delta;

/// @brief `StateImpl` encapsulates the fluent and derived atoms of a planning state.
/// We refer to the fluent atoms as the non-extended state
/// and the fluent and derived atoms as the extended state.
class StateImpl
{
public:
    static const FlatDoubleList s_empty_numeric_variables;  ///< Returned, if m_numeric_variables is a nullptr.

    bool numeric_constraint_holds(formalism::GroundNumericConstraint numeric_constraint, const FlatDoubleList& static_numeric_variables) const;

    bool numeric_constraints_hold(const formalism::GroundNumericConstraintList& numeric_constraints, const FlatDoubleList& static_numeric_variables) const;

    template<formalism::IsFluentOrDerivedTag P>
    bool literals_hold(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;

    /* Getters */

    /// @brief Get the index of the state.
    /// @return
    Index get_index() const;

    /// @brief Get the tree table where the slots are stored.
    /// @return
    const valla::IndexedHashSet& get_tree_table() const;

    /// @brief Get a range over the ground atom indices for the given tag `P` in the state.
    /// @tparam P is the tag.
    /// @return
    template<formalism::IsFluentOrDerivedTag P>
    auto get_atoms() const;

    /// @brief Get the numeric variables in the state.
    /// @return
    const auto& get_numeric_variables() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    ///
    /// Only return the non-extended portion of a state because it implies the extended portion.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        // The pointers uniquely identify the state, derived atoms not needed.
        return std::make_tuple(szudzik_pair(valla::read_pos(m_fluent_atoms, 0), valla::read_pos(m_fluent_atoms, 1)), m_numeric_variables);
    }

private:
    StateImpl(Index index,
              const valla::IndexedHashSet& tree_table,
              valla::Slot fluent_atoms,
              valla::Slot derived_atoms,
              const FlatDoubleList& numeric_variables) :
        m_index(index),
        m_tree_table(&tree_table),
        m_fluent_atoms(fluent_atoms),
        m_derived_atoms(derived_atoms),
        m_numeric_variables(&numeric_variables)
    {
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    friend class StateRepositoryImpl;

    Index m_index;
    /* Propositional variables */
    const valla::IndexedHashSet* m_tree_table;
    valla::Slot m_fluent_atoms;
    valla::Slot m_derived_atoms;
    /* Numeric variables */
    const FlatDoubleList* m_numeric_variables;
};

template<formalism::IsFluentOrDerivedTag P>
auto StateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, formalism::FluentTag>)
    {
        assert(std::is_sorted(v::begin(m_fluent_atoms, get_tree_table()), v::end()));
        return std::ranges::subrange(v::begin(m_fluent_atoms, get_tree_table()), v::end());
    }
    else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
    {
        assert(std::is_sorted(v::begin(m_derived_atoms, get_tree_table()), v::end()));
        return std::ranges::subrange(v::begin(m_derived_atoms, get_tree_table()), v::end());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

inline const auto& StateImpl::get_numeric_variables() const
{
    if (!m_numeric_variables)
    {
        return StateImpl::s_empty_numeric_variables;
    }
    // StateRepositoryImpl ensures that m_numeric_variables is a valid pointer to a FlatDoubleList.
    return *m_numeric_variables;
}

/// @brief STL does not define operator== for std::span.
inline bool operator==(const std::span<const State>& lhs, const std::span<const State>& rhs)
{
    return (lhs.data() == rhs.data()) && (lhs.size() == rhs.size());
}

using StateImplSet = loki::SegmentedRepository<StateImpl>;

}

namespace mimir
{
/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<search::State, const formalism::ProblemImpl&>& data);
}

#endif
