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

#include "mimir/languages/description_logics/evaluation_context.hpp"

namespace mimir::dl
{
EvaluationContext::EvaluationContext(DenotationRepository<Concept>& concept_denotation_repository,
                                     DenotationRepository<Role>& role_denotation_repository,
                                     size_t state_id,
                                     const GroundAtomList<Static>& static_state_atoms,
                                     const GroundAtomList<Fluent>& fluent_state_atoms,
                                     const GroundAtomList<Derived>& derived_state_atoms,
                                     const GroundAtomList<Static>& static_goal_atoms,
                                     const GroundAtomList<Fluent>& fluent_goal_atoms,
                                     const GroundAtomList<Derived>& derived_goal_atoms,
                                     size_t num_objects) :
    m_concept_denotation_repository(concept_denotation_repository),
    m_role_denotation_repository(role_denotation_repository),
    m_state_id(state_id),
    m_static_state_atoms(static_state_atoms),
    m_fluent_state_atoms(fluent_state_atoms),
    m_derived_state_atoms(derived_state_atoms),
    m_static_goal_atoms(static_goal_atoms),
    m_fluent_goal_atoms(fluent_goal_atoms),
    m_derived_goal_atoms(derived_goal_atoms),
    m_concept_denotation_builder(),
    m_role_denotation_builder(num_objects)
{
}

void EvaluationContext::set_state_id(size_t state_id) { m_state_id = state_id; }

template<DynamicPredicateCategory P>
void EvaluationContext::set_state_atoms(const GroundAtomList<P>& state_atoms)
{
    if constexpr (std::is_same_v<P, Fluent>)
    {
        m_fluent_state_atoms = state_atoms;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        m_derived_state_atoms = state_atoms;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template void EvaluationContext::set_state_atoms(const GroundAtomList<Fluent>& state_atoms);
template void EvaluationContext::set_state_atoms(const GroundAtomList<Derived>& state_atoms);

template<IsConceptOrRole D>
DenotationBuilder<D>& EvaluationContext::get_denotation_builder()
{
    if constexpr (std::is_same_v<D, Concept>)
    {
        return m_concept_denotation_builder;
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        return m_role_denotation_builder;
    }
    else
    {
        static_assert(dependent_false<D>::value, "Missing implementation for IsConceptOrRole.");
    }
}

template DenotationBuilder<Concept>& EvaluationContext::get_denotation_builder<Concept>();
template DenotationBuilder<Role>& EvaluationContext::get_denotation_builder<Role>();

template<IsConceptOrRole D>
DenotationRepository<D>& EvaluationContext::get_denotation_repository()
{
    if constexpr (std::is_same_v<D, Concept>)
    {
        return m_concept_denotation_repository;
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        return m_role_denotation_repository;
    }
    else
    {
        static_assert(dependent_false<D>::value, "Missing implementation for IsConceptOrRole.");
    }
}

template DenotationRepository<Concept>& EvaluationContext::get_denotation_repository<Concept>();
template DenotationRepository<Role>& EvaluationContext::get_denotation_repository<Role>();

size_t EvaluationContext::get_state_id() const { return m_state_id; }

template<PredicateCategory P>
const GroundAtomList<P>& EvaluationContext::get_state_atoms() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_state_atoms.get();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_state_atoms.get();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_state_atoms.get();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const GroundAtomList<Static>& EvaluationContext::get_state_atoms<Static>() const;
template const GroundAtomList<Fluent>& EvaluationContext::get_state_atoms<Fluent>() const;
template const GroundAtomList<Derived>& EvaluationContext::get_state_atoms<Derived>() const;

template<PredicateCategory P>
const GroundAtomList<P>& EvaluationContext::get_goal_atoms() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_goal_atoms.get();
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_goal_atoms.get();
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_goal_atoms.get();
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template const GroundAtomList<Static>& EvaluationContext::get_goal_atoms<Static>() const;
template const GroundAtomList<Fluent>& EvaluationContext::get_goal_atoms<Fluent>() const;
template const GroundAtomList<Derived>& EvaluationContext::get_goal_atoms<Derived>() const;
}