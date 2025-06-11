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
#include "mimir/formalism/problem.hpp"
#include "mimir/search/declarations.hpp"

#include <loki/details/utils/equal_to.hpp>
#include <loki/details/utils/hash.hpp>
#include <memory>
#include <valla/canonical_delta_tree_compression.hpp>
#include <valla/canonical_tree_compression.hpp>
#include <valla/delta_tree_compression.hpp>
#include <valla/indexed_hash_set.hpp>
#include <valla/root_slot.hpp>
#include <valla/tree_compression.hpp>

namespace mimir::search
{
namespace v = valla::canonical_delta;

/// @brief `StateImpl` encapsulates the fluent and derived atoms, and numeric variables of a planning state.
class StateImpl
{
private:
    Index m_index;
    const formalism::ProblemImpl* m_problem;
    const valla::RootSlot* m_fluent_atoms;
    const valla::RootSlot* m_derived_atoms;
    const FlatDoubleList* m_numeric_variables;

    StateImpl(Index index,
              const formalism::ProblemImpl* problem,
              const valla::RootSlot* fluent_atoms,
              const valla::RootSlot* derived_atoms,
              const FlatDoubleList* numeric_variables);

    friend class StateRepositoryImpl;

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    /**
     * Getters
     */

    Index get_index() const;
    const formalism::ProblemImpl& get_problem() const;
    template<formalism::IsFluentOrDerivedTag P>
    auto get_atoms() const;
    template<formalism::IsFluentOrDerivedTag P>
    const valla::RootSlot& get_atoms_slot() const;
    const FlatDoubleList& get_numeric_variables() const;

    /**
     * Utils
     */

    /// @brief Check whether the literal holds in the state using binary search over the atoms in the state.
    /// @tparam P is the literal type.
    /// @param literal is the literal.
    /// @return true if the literal holds in the state, and false otherwise.
    template<formalism::IsFluentOrDerivedTag P>
    bool literal_holds(formalism::GroundLiteral<P> literal) const;

    /// @brief Check whether all literals hold in the state using binary searches over the atoms in the state.
    /// @tparam P is the literal type.
    /// @param literals are the literals.
    /// @return true if all literals hold in the state, and false otherwise.
    template<formalism::IsFluentOrDerivedTag P>
    bool literals_hold(const formalism::GroundLiteralList<P>& literals) const;

    bool numeric_constraint_holds(formalism::GroundNumericConstraint numeric_constraint, const FlatDoubleList& static_numeric_variables) const;

    bool numeric_constraints_hold(const formalism::GroundNumericConstraintList& numeric_constraints, const FlatDoubleList& static_numeric_variables) const;

    template<formalism::IsFluentOrDerivedTag P, std::ranges::forward_range Range1, std::ranges::forward_range Range2>
        requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
    bool literals_hold(const Range1& positive_atoms, const Range2& negative_atoms) const;

    auto identifying_members() const { return std::make_tuple(valla::RootSlotHash()(*m_fluent_atoms)); }
};

static_assert(sizeof(StateImpl) == 40);

using StateImplSet = loki::SegmentedRepository<StateImpl>;

/**
 * Implementations
 */

template<formalism::IsFluentOrDerivedTag P>
auto StateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, formalism::FluentTag>)
    {
        return std::ranges::subrange(v::begin(*m_fluent_atoms, m_problem->get_tree_table()), v::end());
    }
    else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
    {
        return std::ranges::subrange(v::begin(*m_derived_atoms, m_problem->get_tree_table()), v::end());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template<formalism::IsFluentOrDerivedTag P>
const valla::RootSlot& StateImpl::get_atoms_slot() const
{
    if constexpr (std::is_same_v<P, formalism::FluentTag>)
    {
        return *m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, formalism::DerivedTag>)
    {
        return *m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template<formalism::IsFluentOrDerivedTag P, std::ranges::forward_range Range1, std::ranges::forward_range Range2>
    requires IsRangeOver<Range1, Index> && IsRangeOver<Range2, Index>
bool StateImpl::literals_hold(const Range1& positive_atoms, const Range2& negative_atoms) const
{
    return is_supseteq(get_atoms<P>(), positive_atoms) && are_disjoint(get_atoms<P>(), negative_atoms);
}

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, search::State state);

}

namespace loki
{
template<>
struct EqualTo<mimir::search::StateImpl>
{
    bool operator()(const mimir::search::StateImpl& lhs, const mimir::search::StateImpl& rhs) const
    {
        return valla::RootSlotEqualTo()(lhs.get_atoms_slot<mimir::formalism::FluentTag>(), rhs.get_atoms_slot<mimir::formalism::FluentTag>());
    }
};
}

#endif
