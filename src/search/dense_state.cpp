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
    insert_into_bitset(state->get_atoms<FluentTag>(), m_fluent_atoms);
    insert_into_bitset(state->get_atoms<DerivedTag>(), m_derived_atoms);
    assert(std::equal(state->get_atoms<FluentTag>().begin(), state->get_atoms<FluentTag>().end(), m_fluent_atoms.begin()));
    assert(std::equal(m_fluent_atoms.begin(), m_fluent_atoms.end(), state->get_atoms<FluentTag>().begin()));
    assert(std::equal(state->get_atoms<DerivedTag>().begin(), state->get_atoms<DerivedTag>().end(), m_derived_atoms.begin()));
    assert(std::equal(m_derived_atoms.begin(), m_derived_atoms.end(), state->get_atoms<DerivedTag>().begin()));
    m_numeric_variables = state->get_numeric_variables();
}

void DenseState::translate(State state, DenseState& out_state)
{
    auto& fluent_atoms = out_state.get_atoms<FluentTag>();
    auto& derived_atoms = out_state.get_atoms<DerivedTag>();
    auto& numeric_variables = out_state.get_numeric_variables();
    fluent_atoms.unset_all();
    derived_atoms.unset_all();
    insert_into_bitset(state->get_atoms<FluentTag>(), fluent_atoms);
    insert_into_bitset(state->get_atoms<DerivedTag>(), derived_atoms);
    assert(std::equal(state->get_atoms<FluentTag>().begin(), state->get_atoms<FluentTag>().end(), fluent_atoms.begin()));
    assert(std::equal(fluent_atoms.begin(), fluent_atoms.end(), state->get_atoms<FluentTag>().begin()));
    assert(std::equal(state->get_atoms<DerivedTag>().begin(), state->get_atoms<DerivedTag>().end(), derived_atoms.begin()));
    assert(std::equal(derived_atoms.begin(), derived_atoms.end(), state->get_atoms<DerivedTag>().begin()));
    numeric_variables = state->get_numeric_variables();
}

template<IsFluentOrDerivedTag P>
bool DenseState::contains(GroundAtom<P> atom) const
{
    return get_atoms<P>().get(atom->get_index());
}

template bool DenseState::contains(GroundAtom<FluentTag> atom) const;
template bool DenseState::contains(GroundAtom<DerivedTag> atom) const;

template<IsFluentOrDerivedTag P>
bool DenseState::literal_holds(GroundLiteral<P> literal) const
{
    return literal->get_polarity() == contains(literal->get_atom());
}

template bool DenseState::literal_holds(GroundLiteral<FluentTag> literal) const;
template bool DenseState::literal_holds(GroundLiteral<DerivedTag> literal) const;

template<IsFluentOrDerivedTag P>
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

Index DenseState::get_index() const { return m_index; }

template<IsFluentOrDerivedTag P>
const FlatBitset& DenseState::get_atoms() const
{
    if constexpr (std::is_same_v<P, FluentTag>)
    {
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template const FlatBitset& DenseState::get_atoms<FluentTag>() const;
template const FlatBitset& DenseState::get_atoms<DerivedTag>() const;

const FlatDoubleList& DenseState::get_numeric_variables() const { return m_numeric_variables; }

Index& DenseState::get_index() { return m_index; }

template<IsFluentOrDerivedTag P>
FlatBitset& DenseState::get_atoms()
{
    if constexpr (std::is_same_v<P, FluentTag>)
    {
        return m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        return m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template FlatBitset& DenseState::get_atoms<FluentTag>();
template FlatBitset& DenseState::get_atoms<DerivedTag>();

FlatDoubleList& DenseState::get_numeric_variables() { return m_numeric_variables; }
}
