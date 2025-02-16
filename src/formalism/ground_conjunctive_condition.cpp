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

#include "mimir/formalism/ground_conjunctive_condition.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/common/hash_cista.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/repositories.hpp"

#include <ostream>
#include <tuple>

namespace mimir
{

/* GroundConjunctiveCondition */

template<StaticOrFluentOrDerived P>
FlatIndexList& GroundConjunctiveCondition::get_positive_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_positive_static_atoms.cbegin(), m_positive_static_atoms.cend()));
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_positive_fluent_atoms.cbegin(), m_positive_fluent_atoms.cend()));
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_positive_derived_atoms.cbegin(), m_positive_derived_atoms.cend()));
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template FlatIndexList& GroundConjunctiveCondition::get_positive_precondition<Static>();
template FlatIndexList& GroundConjunctiveCondition::get_positive_precondition<Fluent>();
template FlatIndexList& GroundConjunctiveCondition::get_positive_precondition<Derived>();

template<StaticOrFluentOrDerived P>
const FlatIndexList& GroundConjunctiveCondition::get_positive_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_positive_static_atoms.begin(), m_positive_static_atoms.end()));
        return m_positive_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_positive_fluent_atoms.begin(), m_positive_fluent_atoms.end()));
        return m_positive_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_positive_derived_atoms.begin(), m_positive_derived_atoms.end()));
        return m_positive_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const FlatIndexList& GroundConjunctiveCondition::get_positive_precondition<Static>() const;
template const FlatIndexList& GroundConjunctiveCondition::get_positive_precondition<Fluent>() const;
template const FlatIndexList& GroundConjunctiveCondition::get_positive_precondition<Derived>() const;

template<StaticOrFluentOrDerived P>
FlatIndexList& GroundConjunctiveCondition::get_negative_precondition()
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_negative_static_atoms.cbegin(), m_negative_static_atoms.cend()));
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_negative_fluent_atoms.cbegin(), m_negative_fluent_atoms.cend()));
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_negative_derived_atoms.cbegin(), m_negative_derived_atoms.cend()));
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template FlatIndexList& GroundConjunctiveCondition::get_negative_precondition<Static>();
template FlatIndexList& GroundConjunctiveCondition::get_negative_precondition<Fluent>();
template FlatIndexList& GroundConjunctiveCondition::get_negative_precondition<Derived>();

template<StaticOrFluentOrDerived P>
const FlatIndexList& GroundConjunctiveCondition::get_negative_precondition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        assert(std::is_sorted(m_negative_static_atoms.begin(), m_negative_static_atoms.end()));
        return m_negative_static_atoms;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        assert(std::is_sorted(m_negative_fluent_atoms.begin(), m_negative_fluent_atoms.end()));
        return m_negative_fluent_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        assert(std::is_sorted(m_negative_derived_atoms.begin(), m_negative_derived_atoms.end()));
        return m_negative_derived_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for StaticOrFluentOrDerived.");
    }
}

template const FlatIndexList& GroundConjunctiveCondition::get_negative_precondition<Static>() const;
template const FlatIndexList& GroundConjunctiveCondition::get_negative_precondition<Fluent>() const;
template const FlatIndexList& GroundConjunctiveCondition::get_negative_precondition<Derived>() const;

FlatExternalPtrList<const GroundNumericConstraintImpl>& GroundConjunctiveCondition::get_numeric_constraints() { return m_numeric_constraints; }

const FlatExternalPtrList<const GroundNumericConstraintImpl>& GroundConjunctiveCondition::get_numeric_constraints() const { return m_numeric_constraints; }

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<GroundConjunctiveCondition, const PDDLRepositories&>& data)
{
    // TODO(numerical): add numeric constraints
    const auto& [conjunctive_condition, pddl_repositories] = data;

    const auto& positive_static_precondition_indices = conjunctive_condition.get_positive_precondition<Static>();
    const auto& negative_static_precondition_indices = conjunctive_condition.get_negative_precondition<Static>();
    const auto& positive_fluent_precondition_indices = conjunctive_condition.get_positive_precondition<Fluent>();
    const auto& negative_fluent_precondition_indices = conjunctive_condition.get_negative_precondition<Fluent>();
    const auto& positive_derived_precondition_indices = conjunctive_condition.get_positive_precondition<Derived>();
    const auto& negative_derived_precondition_indices = conjunctive_condition.get_negative_precondition<Derived>();

    auto positive_static_precondition = GroundAtomList<Static> {};
    auto negative_static_precondition = GroundAtomList<Static> {};
    auto positive_fluent_precondition = GroundAtomList<Fluent> {};
    auto negative_fluent_precondition = GroundAtomList<Fluent> {};
    auto positive_derived_precondition = GroundAtomList<Derived> {};
    auto negative_derived_precondition = GroundAtomList<Derived> {};
    const auto& ground_numeric_constraints = conjunctive_condition.get_numeric_constraints();

    pddl_repositories.get_ground_atoms_from_indices<Static>(positive_static_precondition_indices, positive_static_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Static>(negative_static_precondition_indices, negative_static_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(positive_fluent_precondition_indices, positive_fluent_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Fluent>(negative_fluent_precondition_indices, negative_fluent_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Derived>(positive_derived_precondition_indices, positive_derived_precondition);
    pddl_repositories.get_ground_atoms_from_indices<Derived>(negative_derived_precondition_indices, negative_derived_precondition);

    os << "positive static precondition=" << positive_static_precondition << ", " << "negative static precondition=" << negative_static_precondition << ", "
       << "positive fluent precondition=" << positive_fluent_precondition << ", " << "negative fluent precondition=" << negative_fluent_precondition << ", "
       << "positive derived precondition=" << positive_derived_precondition << ", " << "negative derived precondition=" << negative_derived_precondition << ", "
       << "numeric constraints=" << ground_numeric_constraints;

    return os;
}

}
