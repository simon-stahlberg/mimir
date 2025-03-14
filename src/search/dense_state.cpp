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

#include "mimir/search/dense_state.hpp"

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/search/state.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

DenseState::DenseState(State state)
{
    insert_into_bitset(state->get_atoms<Fluent>(), m_fluent_atoms);
    insert_into_bitset(state->get_atoms<Derived>(), m_derived_atoms);
    m_numeric_variables = state->get_numeric_variables();
}

void DenseState::translate(State state, DenseState& out_state)
{
    auto& fluent_atoms = out_state.get_atoms<Fluent>();
    auto& derived_atoms = out_state.get_atoms<Derived>();
    auto& numeric_variables = out_state.get_numeric_variables();
    fluent_atoms.unset_all();
    derived_atoms.unset_all();
    insert_into_bitset(state->get_atoms<Fluent>(), fluent_atoms);
    insert_into_bitset(state->get_atoms<Derived>(), derived_atoms);
    numeric_variables = state->get_numeric_variables();
}

template<FluentOrDerived P>
bool DenseState::contains(GroundAtom<P> atom) const
{
    return get_atoms<P>().get(atom->get_index());
}

template bool DenseState::contains(GroundAtom<Fluent> atom) const;
template bool DenseState::contains(GroundAtom<Derived> atom) const;

template<FluentOrDerived P>
bool DenseState::literal_holds(GroundLiteral<P> literal) const
{
    return literal->is_negated() != contains(literal->get_atom());
}

template bool DenseState::literal_holds(GroundLiteral<Fluent> literal) const;
template bool DenseState::literal_holds(GroundLiteral<Derived> literal) const;

template<FluentOrDerived P>
bool DenseState::literals_hold(const GroundLiteralList<P>& literals) const
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

template<FluentOrDerived P>
bool DenseState::literals_hold(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const
{
    return is_supseteq(get_atoms<P>(), positive_atoms) && are_disjoint(get_atoms<P>(), negative_atoms);
}

template bool DenseState::literals_hold<Fluent>(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;
template bool DenseState::literals_hold<Derived>(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;

Index DenseState::get_index() const { return m_index; }

template<FluentOrDerived P>
const FlatBitset& DenseState::get_atoms() const
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const FlatBitset& DenseState::get_atoms<Fluent>() const;
template const FlatBitset& DenseState::get_atoms<Derived>() const;

const FlatDoubleList& DenseState::get_numeric_variables() const { return m_numeric_variables; }

Index& DenseState::get_index() { return m_index; }

template<FluentOrDerived P>
FlatBitset& DenseState::get_atoms()
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template FlatBitset& DenseState::get_atoms<Fluent>();
template FlatBitset& DenseState::get_atoms<Derived>();

FlatDoubleList& DenseState::get_numeric_variables() { return m_numeric_variables; }
}
