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

#include "mimir/search/state.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/repositories.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* State */

const FlatIndexList StateImpl::s_empty_derived_atoms = FlatIndexList();

template<DynamicPredicateTag P>
bool StateImpl::literal_holds(GroundLiteral<P> literal) const
{
    return literal->is_negated() != contains(get_atoms<P>(), literal->get_atom()->get_index());
}

template bool StateImpl::literal_holds(GroundLiteral<Fluent> literal) const;
template bool StateImpl::literal_holds(GroundLiteral<Derived> literal) const;

template<DynamicPredicateTag P>
bool StateImpl::literals_hold(const GroundLiteralList<P>& literals) const
{
    for (const auto& literal : literals)
    {
        if (!literal_holds(literal))
        {
            return false;
        }
    }

    return true;
}

template bool StateImpl::literals_hold(const GroundLiteralList<Fluent>& literals) const;
template bool StateImpl::literals_hold(const GroundLiteralList<Derived>& literals) const;

template<DynamicPredicateTag P>
bool StateImpl::literals_hold(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const
{
    return is_supseteq(get_atoms<P>(), positive_atoms) && are_disjoint(get_atoms<P>(), negative_atoms);
}

template bool StateImpl::literals_hold<Fluent>(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;
template bool StateImpl::literals_hold<Derived>(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;

Index StateImpl::get_index() const { return m_index; }

template<DynamicPredicateTag P>
const FlatIndexList& StateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_fluent_atoms.begin(), m_fluent_atoms.end()));
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        if (!m_derived_atoms)
        {
            return StateImpl::s_empty_derived_atoms;
        }
        // StateRepository ensures that m_derived_atoms is a valid pointer to a FlatIndexList.
        const auto& derived_atoms = *reinterpret_cast<const FlatIndexList*>(m_derived_atoms);
        assert(std::is_sorted(derived_atoms.begin(), derived_atoms.end()));
        return derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateTag.");
    }
}

template const FlatIndexList& StateImpl::get_atoms<Fluent>() const;
template const FlatIndexList& StateImpl::get_atoms<Derived>() const;

Index& StateImpl::get_index() { return m_index; }

FlatIndexList& StateImpl::get_fluent_atoms()
{
    assert(std::is_sorted(get_atoms<Fluent>().begin(), get_atoms<Fluent>().end()));
    return m_fluent_atoms;
}

uintptr_t& StateImpl::get_derived_atoms() { return m_derived_atoms; }

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, State, const PDDLRepositories&>& data)
{
    const auto [problem, state, pddl_repositories] = data;

    auto out_fluent_ground_atoms = GroundAtomList<Fluent> {};
    auto out_static_ground_atoms = GroundAtomList<Static> {};
    auto out_derived_ground_atoms = GroundAtomList<Derived> {};

    pddl_repositories.get_ground_atoms_from_indices(state->get_atoms<Fluent>(), out_fluent_ground_atoms);
    pddl_repositories.get_ground_atoms_from_indices(problem->get_static_initial_positive_atoms_bitset(), out_static_ground_atoms);
    pddl_repositories.get_ground_atoms_from_indices(state->get_atoms<Derived>(), out_derived_ground_atoms);

    // Sort by name for easier comparison
    std::sort(out_fluent_ground_atoms.begin(),
              out_fluent_ground_atoms.end(),
              [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(out_static_ground_atoms.begin(),
              out_static_ground_atoms.end(),
              [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(out_derived_ground_atoms.begin(),
              out_derived_ground_atoms.end(),
              [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });

    os << "State("
       << "index=" << state->get_index() << ", "
       << "fluent atoms=" << out_fluent_ground_atoms << ", "
       << "static atoms=" << out_static_ground_atoms << ", "
       << "derived atoms=" << out_derived_ground_atoms << ")";

    return os;
}
}
