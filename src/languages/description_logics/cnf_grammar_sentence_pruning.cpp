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

#include "mimir/languages/description_logics/cnf_grammar_sentence_pruning.hpp"

#include "mimir/formalism/problem.hpp"

namespace mimir::dl
{

/**
 * RefinementStateListPruningFunction
 */

RefinementStateListPruningFunction::RefinementStateListPruningFunction(ProblemMap<StateList> state_partitioning) :
    RefinementPruningFunction(),
    m_state_partitioning(state_partitioning),
    m_repositories()
{
}

bool RefinementStateListPruningFunction::should_prune(Constructor<Concept> concept_) { return should_prune_impl(concept_); }

bool RefinementStateListPruningFunction::should_prune(Constructor<Role> role_) { return should_prune_impl(role_); }

template<ConceptOrRole D>
bool RefinementStateListPruningFunction::should_prune_impl(Constructor<D> constructor)
{
    auto denotations = DenotationsList();

    for (const auto& [problem, states] : m_state_partitioning)
    {
        for (const auto& state : states)
        {
            auto builders = Denotations<Concept, Role>();

            // We resize role denotation here to avoid having to do that during evaluation.
            boost::hana::at_key(builders, boost::hana::type<Role> {}).get_data().resize(problem->get_objects().size());

            auto evaluation_context = EvaluationContext(state, problem, builders, m_repositories);

            const auto eval = constructor->evaluate(evaluation_context);

            denotations.push_back(reinterpret_cast<uintptr_t>(eval));
        }
    }

    return !m_denotations_repository.insert(denotations).second;
}

template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<Concept> constructor);
template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<Role> constructor);

}
