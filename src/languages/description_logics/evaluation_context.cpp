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
EvaluationContext::EvaluationContext(State state,
                                     Problem problem,
                                     Denotations<Concept, Role>& ref_builders,
                                     DenotationRepositories<Concept, Role>& ref_repositories) :
    m_state(state),
    m_problem(problem),
    m_builders(ref_builders),
    m_repositories(ref_repositories)
{
}

State EvaluationContext::get_state() const { return m_state; }

Problem EvaluationContext::get_problem() const { return m_problem; }

template<ConceptOrRole D>
DenotationImpl<D>& EvaluationContext::get_builder()
{
    return boost::hana::at_key(m_builders, boost::hana::type<D> {});
}

template DenotationImpl<Concept>& EvaluationContext::get_builder<Concept>();
template DenotationImpl<Role>& EvaluationContext::get_builder<Role>();

template<ConceptOrRole D>
DenotationRepository<D>& EvaluationContext::get_repository()
{
    return boost::hana::at_key(m_repositories, boost::hana::type<D> {});
}

template DenotationRepository<Concept>& EvaluationContext::get_repository<Concept>();
template DenotationRepository<Role>& EvaluationContext::get_repository<Role>();

}