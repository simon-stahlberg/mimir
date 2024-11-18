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
EvaluationContext::EvaluationContext(const PDDLRepositories& pddl_repositories,
                                     Problem problem,
                                     State state,
                                     ConstructorTagToDenotationType& ref_denotation_builder,
                                     ConstructorTagToDenotationRepository& ref_denotation_repository) :
    m_pddl_repositories(pddl_repositories),
    m_problem(problem),
    m_state(state),
    m_denotation_builder(ref_denotation_builder),
    m_denotation_repository(ref_denotation_repository)
{
}

const PDDLRepositories& EvaluationContext::get_pddl_repositories() const { return m_pddl_repositories; }

Problem EvaluationContext::get_problem() const { return m_problem; }

State EvaluationContext::get_state() const { return m_state; }

template<ConstructorTag D>
DenotationImpl<D>& EvaluationContext::get_denotation_builder()
{
    return boost::hana::at_key(m_denotation_builder, boost::hana::type<D> {});
}

template DenotationImpl<Concept>& EvaluationContext::get_denotation_builder<Concept>();
template DenotationImpl<Role>& EvaluationContext::get_denotation_builder<Role>();

template<ConstructorTag D>
DenotationRepository<D>& EvaluationContext::get_denotation_repository()
{
    return boost::hana::at_key(m_denotation_repository, boost::hana::type<D> {});
}

template DenotationRepository<Concept>& EvaluationContext::get_denotation_repository<Concept>();
template DenotationRepository<Role>& EvaluationContext::get_denotation_repository<Role>();

}