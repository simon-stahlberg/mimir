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

#include "mimir/languages/description_logics/refinement.hpp"

#include "mimir/formalism/problem.hpp"

namespace mimir::dl
{

/**
 * RefinementStateListPruningFunction
 */

RefinementStateListPruningFunction::RefinementStateListPruningFunction(const PDDLRepositories& pddl_repositories, Problem problem, StateList states) :
    RefinementPruningFunction(),
    m_pddl_repositories(pddl_repositories),
    m_problem(problem),
    m_states(std::move(states)),
    m_denotation_builder(),
    m_denotation_repository()
{
    // Resize role denotation.
    boost::hana::at_key(m_denotation_builder, boost::hana::type<Role> {}).get_data().resize(m_problem->get_objects().size());
}

bool RefinementStateListPruningFunction::should_prune(Constructor<Concept> concept_) { return should_prune_impl(concept_); }

bool RefinementStateListPruningFunction::should_prune(Constructor<Role> role_) { return should_prune_impl(role_); }

template<ConstructorTag D>
bool RefinementStateListPruningFunction::should_prune_impl(Constructor<D> constructor)
{
    auto denotations = DenotationsList();
    denotations.reserve(m_states.size());

    for (const auto& state : m_states)
    {
        auto evaluation_context = EvaluationContext(m_pddl_repositories, m_problem, state, m_denotation_builder, m_denotation_repository);

        const auto eval = constructor->evaluate(evaluation_context);

        denotations.push_back(reinterpret_cast<uintptr_t>(eval));
    }

    return !m_denotations_repository.insert(denotations).second;
}

template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<Concept> constructor);
template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<Role> constructor);

}
