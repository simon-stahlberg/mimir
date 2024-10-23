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
#include "mimir/common/hash.hpp"
#include "mimir/formalism/factories.hpp"

#include <ostream>
#include <tuple>

size_t cista::storage::DerefStdHasher<mimir::FlatState>::operator()(const mimir::FlatState* ptr) const { return mimir::hash_combine(cista::get<1>(*ptr)); }

bool cista::storage::DerefStdEqualTo<mimir::FlatState>::operator()(const mimir::FlatState* lhs, const mimir::FlatState* rhs) const
{
    return cista::get<1>(*lhs) == cista::get<1>(*rhs);
}

size_t std::hash<mimir::State>::operator()(mimir::State element) const { return element.get_index(); }

namespace mimir
{

/* StateBuilder */

StateBuilder::StateBuilder() : m_data() {}

Index& StateBuilder::get_index() { return cista::get<0>(m_data); }

template<DynamicPredicateCategory P>
FlatBitset& StateBuilder::get_atoms()
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<1>(m_data);
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<2>(m_data);
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitset& StateBuilder::get_atoms<Fluent>();
template FlatBitset& StateBuilder::get_atoms<Derived>();

FlatState& StateBuilder::get_data() { return m_data; }
const FlatState& StateBuilder::get_data() const { return m_data; }

/* State */
State::State(const FlatState& data) : m_data(data) {}

Index State::get_index() const { return cista::get<0>(m_data.get()); }

template<DynamicPredicateCategory P>
bool State::contains(GroundAtom<P> atom) const
{
    return get_atoms<P>().get(atom->get_index());
}

template bool State::contains(GroundAtom<Fluent> atom) const;
template bool State::contains(GroundAtom<Derived> atom) const;

template<DynamicPredicateCategory P>
bool State::superset_of(const GroundAtomList<P>& atoms) const
{
    for (const auto& atom : atoms)
    {
        if (!contains(atom))
        {
            return false;
        }
    }

    return true;
}

template bool State::superset_of(const GroundAtomList<Fluent>& atoms) const;
template bool State::superset_of(const GroundAtomList<Derived>& atoms) const;

template<DynamicPredicateCategory P>
bool State::literal_holds(GroundLiteral<P> literal) const
{
    return literal->is_negated() != contains(literal->get_atom());
}

template bool State::literal_holds(GroundLiteral<Fluent> literal) const;
template bool State::literal_holds(GroundLiteral<Derived> literal) const;

template<DynamicPredicateCategory P>
bool State::literals_hold(const GroundLiteralList<P>& literals) const
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

template bool State::literals_hold(const GroundLiteralList<Fluent>& literals) const;
template bool State::literals_hold(const GroundLiteralList<Derived>& literals) const;

template<DynamicPredicateCategory P>
const FlatBitset& State::get_atoms() const
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return cista::get<1>(m_data.get());
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return cista::get<2>(m_data.get());
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for DynamicPredicateCategory.");
    }
}

template const FlatBitset& State::get_atoms<Fluent>() const;
template const FlatBitset& State::get_atoms<Derived>() const;

bool operator==(State lhs, State rhs) { return lhs.get_index() == rhs.get_index(); }
bool operator!=(State lhs, State rhs) { return !(lhs == rhs); }

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, State, const PDDLFactories&>& data)
{
    const auto [problem, state, pddl_factories] = data;

    auto out_fluent_ground_atoms = GroundAtomList<Fluent> {};
    auto out_static_ground_atoms = GroundAtomList<Static> {};
    auto out_derived_ground_atoms = GroundAtomList<Derived> {};

    pddl_factories.get_ground_atoms_from_indices(state.get_atoms<Fluent>(), out_fluent_ground_atoms);
    pddl_factories.get_ground_atoms_from_indices(problem->get_static_initial_positive_atoms(), out_static_ground_atoms);
    pddl_factories.get_ground_atoms_from_indices(state.get_atoms<Derived>(), out_derived_ground_atoms);

    os << "State("
       << "index=" << state.get_index() << ", "
       << "fluent atoms=" << out_fluent_ground_atoms << ", "
       << "static atoms=" << out_static_ground_atoms << ", "
       << "derived atoms=" << out_derived_ground_atoms << ")";

    return os;
}
}
