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
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"

#include <ostream>
#include <tuple>

using namespace mimir::formalism;

namespace mimir::search
{

/* State */

StateImpl::StateImpl(Index index, const FlatIndexList* fluent_atoms, const FlatIndexList* derived_atoms, const FlatDoubleList* numeric_variables) :
    m_index(index),
    m_fluent_atoms(fluent_atoms),
    m_derived_atoms(derived_atoms),
    m_numeric_variables(numeric_variables)
{
    assert(std::is_sorted(m_fluent_atoms->compressed_begin(), m_fluent_atoms->compressed_end()));
    assert(std::is_sorted(m_derived_atoms->compressed_begin(), m_derived_atoms->compressed_end()));
}

template<IsFluentOrDerivedTag P>
bool StateImpl::literal_holds(GroundLiteral<P> literal) const
{
    return (std::find(get_atoms<P>().begin(), get_atoms<P>().end(), literal->get_index()) != get_atoms<P>().end()) == literal->get_polarity();
}

template bool StateImpl::literal_holds(GroundLiteral<FluentTag> literal) const;
template bool StateImpl::literal_holds(GroundLiteral<DerivedTag> literal) const;

template<IsFluentOrDerivedTag P>
bool StateImpl::literals_hold(const GroundLiteralList<P>& literals) const
{
    return std::all_of(literals.begin(), literals.end(), [this](auto&& arg) { return this->literal_holds(arg); });
}

template bool StateImpl::literals_hold(const GroundLiteralList<FluentTag>& literals) const;
template bool StateImpl::literals_hold(const GroundLiteralList<DerivedTag>& literals) const;

bool StateImpl::numeric_constraint_holds(GroundNumericConstraint numeric_constraint, const FlatDoubleList& static_numeric_variables) const
{
    return evaluate(numeric_constraint, static_numeric_variables, get_numeric_variables());
}

bool StateImpl::numeric_constraints_hold(const GroundNumericConstraintList& numeric_constraints, const FlatDoubleList& static_numeric_variables) const
{
    return std::all_of(numeric_constraints.begin(),
                       numeric_constraints.end(),
                       [this, &static_numeric_variables](auto&& arg) { return this->numeric_constraint_holds(arg, static_numeric_variables); });
}

Index StateImpl::get_index() const { return m_index; }

const FlatDoubleList& StateImpl::get_numeric_variables() const { return *m_numeric_variables; }

}

namespace mimir
{
/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<search::State, const ProblemImpl&>& data)
{
    const auto& [state, problem] = data;

    auto fluent_ground_atoms = GroundAtomList<FluentTag> {};
    auto static_ground_atoms = GroundAtomList<StaticTag> {};
    auto derived_ground_atoms = GroundAtomList<DerivedTag> {};
    auto fluent_function_values = std::vector<std::pair<GroundFunction<FluentTag>, ContinuousCost>> {};

    problem.get_repositories().get_ground_atoms_from_indices(state->get_atoms<FluentTag>(), fluent_ground_atoms);
    problem.get_repositories().get_ground_atoms_from_indices(problem.get_static_initial_positive_atoms_bitset(), static_ground_atoms);
    problem.get_repositories().get_ground_atoms_from_indices(state->get_atoms<DerivedTag>(), derived_ground_atoms);
    problem.get_repositories().get_ground_function_values(state->get_numeric_variables(), fluent_function_values);

    // Sort by name for easier comparison
    std::sort(fluent_ground_atoms.begin(), fluent_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(static_ground_atoms.begin(), static_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(derived_ground_atoms.begin(), derived_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });

    os << "State(" << "index=" << state->get_index() << ", " << "fluent atoms=" << fluent_ground_atoms << ", " << "static atoms=" << static_ground_atoms << ", "
       << "derived atoms=" << derived_ground_atoms << ", " << "fluent numerics=" << fluent_function_values << ")";

    return os;
}
}
