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
#include "mimir/formalism/factories.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* FlatStateHash*/

size_t FlatStateHash::operator()(FlatState view) const
{
    const auto fluent_atoms = view.get<1>();
    return loki::hash_combine(fluent_atoms.hash());
}

/* FlatStateEqual */

bool FlatStateEqual::operator()(FlatState view_left, FlatState view_right) const
{
    const auto fluent_atoms_left = view_left.get<1>();
    const auto fluent_atoms_right = view_right.get<1>();
    return (fluent_atoms_left == fluent_atoms_right);
}

/* StateBuilder */

FlatStateBuilder& StateBuilder::get_flatmemory_builder() { return m_builder; }
const FlatStateBuilder& StateBuilder::get_flatmemory_builder() const { return m_builder; }

StateId& StateBuilder::get_id() { return m_builder.get<0>(); }

template<DynamicPredicateCategory P>
FlatBitsetBuilder<P>& StateBuilder::get_atoms()
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_builder.get<1>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_builder.get<2>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitsetBuilder<Fluent>& StateBuilder::get_atoms();
template FlatBitsetBuilder<Derived>& StateBuilder::get_atoms();

/* State */

State::State(FlatState view) : m_view(view) {}

bool State::operator==(State other) const { return m_view.buffer() == other.m_view.buffer(); }

size_t State::hash() const { return loki::hash_combine(m_view.buffer()); }

StateId State::get_id() const { return m_view.get<0>(); }

template<DynamicPredicateCategory P>
bool State::contains(GroundAtom<P> atom) const
{
    return get_atoms<P>().get(atom->get_identifier());
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
FlatBitset<P> State::get_atoms() const
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_view.get<1>();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_view.get<2>();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template FlatBitset<Fluent> State::get_atoms<Fluent>() const;
template FlatBitset<Derived> State::get_atoms<Derived>() const;

/* StateHash */

size_t StateHash::operator()(State view) const { return view.hash(); }

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const std::tuple<Problem, State, const PDDLFactories&>& data)
{
    const auto [problem, state, pddl_factories] = data;

    auto out_fluent_ground_atoms = GroundAtomList<Fluent> {};
    auto out_static_ground_atoms = GroundAtomList<Static> {};
    auto out_derived_ground_atoms = GroundAtomList<Derived> {};

    pddl_factories.get_ground_atoms_from_ids(state.get_atoms<Fluent>(), out_fluent_ground_atoms);
    pddl_factories.get_ground_atoms_from_ids(problem->get_static_initial_positive_atoms_bitset(), out_static_ground_atoms);
    pddl_factories.get_ground_atoms_from_ids(state.get_atoms<Derived>(), out_derived_ground_atoms);

    os << "State("
       << "state id=" << state.get_id() << ", "
       << "fluent atoms=" << out_fluent_ground_atoms << ", "
       << "static atoms=" << out_static_ground_atoms << ", "
       << "derived atoms=" << out_derived_ground_atoms << ")";

    return os;
}

}
