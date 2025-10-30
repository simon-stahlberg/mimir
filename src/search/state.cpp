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

#include "mimir/common/declarations.hpp"
#include "mimir/common/formatter.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <ostream>
#include <tuple>

using namespace mimir::formalism;

namespace mimir::search
{

State::State(Index index, PackedState packed, SharedObjectPoolPtr<UnpackedStateImpl> unpacked, StateRepository parent) :
    m_parent(std::move(parent)),
    m_packed(packed),
    m_unpacked(std::move(unpacked)),
    m_index(index)
{
    assert(m_packed && m_unpacked);
    assert(std::is_sorted(get_atoms<FluentTag>().begin(), get_atoms<FluentTag>().end()));
    assert(std::is_sorted(get_atoms<DerivedTag>().begin(), get_atoms<DerivedTag>().end()));
}

bool State::operator==(const State& other) const noexcept { return loki::EqualTo<State> {}(*this, other); }

bool State::operator!=(const State& other) const noexcept { return !(*this == other); }

Index State::get_index() const { return m_index; }

const StateRepository& State::get_state_repository() const { return m_parent; }

PackedState State::get_packed_state() const { return m_packed; }

const UnpackedStateImpl& State::get_unpacked_state() const { return *m_unpacked; }

const ProblemImpl& State::get_problem() const { return m_unpacked->get_problem(); }

template<IsFluentOrDerivedTag P>
bool State::literal_holds(GroundLiteral<P> literal) const
{
    return (std::find(get_atoms<P>().begin(), get_atoms<P>().end(), literal->get_atom()->get_index()) != get_atoms<P>().end()) == literal->get_polarity();
}

template bool State::literal_holds(GroundLiteral<FluentTag> literal) const;
template bool State::literal_holds(GroundLiteral<DerivedTag> literal) const;

template<IsFluentOrDerivedTag P>
bool State::literals_hold(const GroundLiteralList<P>& literals) const
{
    return std::all_of(literals.begin(), literals.end(), [this](auto&& arg) { return this->literal_holds(arg); });
}

template bool State::literals_hold(const GroundLiteralList<FluentTag>& literals) const;
template bool State::literals_hold(const GroundLiteralList<DerivedTag>& literals) const;

bool State::numeric_constraint_holds(GroundNumericConstraint numeric_constraint, const FlatDoubleList& static_numeric_variables) const
{
    return evaluate(numeric_constraint, static_numeric_variables, m_unpacked->get_numeric_variables());
}

bool State::numeric_constraints_hold(const GroundNumericConstraintList& numeric_constraints, const FlatDoubleList& static_numeric_variables) const
{
    return std::all_of(numeric_constraints.begin(),
                       numeric_constraints.end(),
                       [this, &static_numeric_variables](auto&& arg) { return this->numeric_constraint_holds(arg, static_numeric_variables); });
}

template<IsFluentOrDerivedTag P>
const FlatBitset& State::get_atoms() const
{
    return m_unpacked->get_atoms<P>();
}

template const FlatBitset& State::get_atoms<FluentTag>() const;
template const FlatBitset& State::get_atoms<DerivedTag>() const;

const FlatDoubleList& State::get_numeric_variables() const { return m_unpacked->get_numeric_variables(); }

}

namespace loki
{
size_t Hash<mimir::search::State>::operator()(const mimir::search::State& el) const
{
    return hash_combine(Hash<mimir::search::PackedStateImpl> {}(*el.get_packed_state()), &el.get_problem());
}

bool EqualTo<mimir::search::State>::operator()(const mimir::search::State& lhs, const mimir::search::State& rhs) const
{
    return EqualTo<mimir::search::PackedStateImpl> {}(*lhs.get_packed_state(), *rhs.get_packed_state()) && &lhs.get_problem() == &rhs.get_problem();
}

}
