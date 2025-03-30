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

const FlatIndexList StateImpl::s_empty_fluent_atoms = FlatIndexList();

const FlatIndexList StateImpl::s_empty_derived_atoms = FlatIndexList();

const FlatDoubleList StateImpl::s_empty_numeric_variables = FlatDoubleList();

bool StateImpl::numeric_constraint_holds(GroundNumericConstraint numeric_constraint, const FlatDoubleList& static_numeric_variables) const
{
    return evaluate(numeric_constraint, static_numeric_variables, get_numeric_variables());
}

bool StateImpl::numeric_constraints_hold(const GroundNumericConstraintList& numeric_constraints, const FlatDoubleList& static_numeric_variables) const
{
    for (const auto& constraint : numeric_constraints)
    {
        if (!numeric_constraint_holds(constraint, static_numeric_variables))
        {
            return false;
        }
    }
    return true;
}

template<IsFluentOrDerivedTag P>
bool StateImpl::literal_holds(GroundLiteral<P> literal) const
{
    return literal->get_polarity() == contains(get_atoms<P>(), literal->get_atom()->get_index());
}

template bool StateImpl::literal_holds(GroundLiteral<FluentTag> literal) const;
template bool StateImpl::literal_holds(GroundLiteral<DerivedTag> literal) const;

template<IsFluentOrDerivedTag P>
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

template bool StateImpl::literals_hold(const GroundLiteralList<FluentTag>& literals) const;
template bool StateImpl::literals_hold(const GroundLiteralList<DerivedTag>& literals) const;

template<IsFluentOrDerivedTag P>
bool StateImpl::literals_hold(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const
{
    return is_supseteq(get_atoms<P>(), positive_atoms) && are_disjoint(get_atoms<P>(), negative_atoms);
}

template bool StateImpl::literals_hold<FluentTag>(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;
template bool StateImpl::literals_hold<DerivedTag>(const FlatIndexList& positive_atoms, const FlatIndexList& negative_atoms) const;

Index StateImpl::get_index() const { return m_index; }

template<IsFluentOrDerivedTag P>
const FlatIndexList& StateImpl::get_atoms() const
{
    if constexpr (std::is_same_v<P, FluentTag>)
    {
        if (!m_fluent_atoms)
        {
            return StateImpl::s_empty_fluent_atoms;
        }
        assert(std::is_sorted(m_fluent_atoms->begin(), m_fluent_atoms->end()));
        return *m_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, DerivedTag>)
    {
        if (!m_derived_atoms)
        {
            return StateImpl::s_empty_derived_atoms;
        }
        // StateRepositoryImpl ensures that m_derived_atoms is a valid pointer to a FlatIndexList.
        assert(std::is_sorted(m_derived_atoms->begin(), m_derived_atoms->end()));
        return *m_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for IsStaticOrFluentOrDerivedTag.");
    }
}

template const FlatIndexList& StateImpl::get_atoms<FluentTag>() const;
template const FlatIndexList& StateImpl::get_atoms<DerivedTag>() const;

const FlatDoubleList& StateImpl::get_numeric_variables() const
{
    if (!m_numeric_variables)
    {
        return StateImpl::s_empty_numeric_variables;
    }
    // StateRepositoryImpl ensures that m_numeric_variables is a valid pointer to a FlatDoubleList.
    return *m_numeric_variables;
}

Index& StateImpl::get_index() { return m_index; }

FlatExternalPtr<const FlatIndexList>& StateImpl::get_fluent_atoms() { return m_fluent_atoms; }

FlatExternalPtr<const FlatIndexList>& StateImpl::get_derived_atoms() { return m_derived_atoms; }

FlatExternalPtr<const FlatDoubleList>& StateImpl::get_numeric_variables() { return m_numeric_variables; }

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
