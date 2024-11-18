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
EvaluationContext::EvaluationContext(const PDDLRepositories& pddl_factories,
                                     Problem problem,
                                     State state,
                                     DenotationImpl<Concept>& ref_concept_denotation_builder,
                                     DenotationImpl<Role>& ref_role_denotation_builder,
                                     DenotationRepository<Concept>& ref_concept_denotation_repository,
                                     DenotationRepository<Role>& ref_role_denotation_repository) :
    m_pddl_factories(pddl_factories),
    m_problem(problem),
    m_state(state),
    m_concept_denotation_builder(ref_concept_denotation_builder),
    m_role_denotation_builder(ref_role_denotation_builder),
    m_concept_denotation_repository(ref_concept_denotation_repository),
    m_role_denotation_repository(ref_role_denotation_repository)
{
}

const PDDLRepositories& EvaluationContext::get_pddl_repositories() const { return m_pddl_factories; }

Problem EvaluationContext::get_problem() const { return m_problem; }

State EvaluationContext::get_state() const { return m_state; }

template<ConstructorTag D>
DenotationImpl<D>& EvaluationContext::get_denotation_builder()
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
        static_assert(dependent_false<D>::value, "Missing implementation for ConstructorTag.");
    }
}

template DenotationImpl<Concept>& EvaluationContext::get_denotation_builder<Concept>();
template DenotationImpl<Role>& EvaluationContext::get_denotation_builder<Role>();

template<ConstructorTag D>
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
        static_assert(dependent_false<D>::value, "Missing implementation for ConstructorTag.");
    }
}

template DenotationRepository<Concept>& EvaluationContext::get_denotation_repository<Concept>();
template DenotationRepository<Role>& EvaluationContext::get_denotation_repository<Role>();

}