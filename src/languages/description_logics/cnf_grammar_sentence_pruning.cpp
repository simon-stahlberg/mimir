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

#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/formalism/problem.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl
{

/**
 * RefinementStateListPruningFunction
 */

RefinementStateListPruningFunction::RefinementStateListPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space) :
    RefinementStateListPruningFunction(generalized_state_space, generalized_state_space->get_graph())
{
}

RefinementStateListPruningFunction::RefinementStateListPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space,
                                                                       const graphs::ClassGraph& class_graph)
{
    for (const auto& vertex : class_graph.get_vertices())
    {
        const auto& problem = graphs::get_problem(generalized_state_space->get_problem_vertex(vertex));
        const auto& state = graphs::get_state(generalized_state_space->get_problem_vertex(vertex));

        m_state_partitioning[problem].push_back(state);
    }
}

RefinementStateListPruningFunction::RefinementStateListPruningFunction(ProblemMap<search::StateList> state_partitioning) :
    RefinementPruningFunction(),
    m_state_partitioning(state_partitioning),
    m_repositories()
{
}

bool RefinementStateListPruningFunction::should_prune(Constructor<ConceptTag> concept_) { return should_prune_impl(concept_); }

bool RefinementStateListPruningFunction::should_prune(Constructor<RoleTag> role) { return should_prune_impl(role); }

bool RefinementStateListPruningFunction::should_prune(Constructor<BooleanTag> boolean) { return should_prune_impl(boolean); }

bool RefinementStateListPruningFunction::should_prune(Constructor<NumericalTag> numerical) { return should_prune_impl(numerical); }

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
bool RefinementStateListPruningFunction::should_prune_impl(Constructor<D> constructor)
{
    auto denotations = DenotationList<D>();

    for (const auto& [problem, states] : m_state_partitioning)
    {
        for (const auto& state : states)
        {
            auto evaluation_context = EvaluationContext(state, problem, m_repositories);

            const auto denotation = constructor->evaluate(evaluation_context);

            denotations.push_back(denotation);
        }
    }

    const auto inserted = boost::hana::at_key(m_denotations_repository, boost::hana::type<D> {}).insert(denotations).second;

    return !inserted;
}

template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<ConceptTag> constructor);
template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<RoleTag> constructor);
template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<BooleanTag> constructor);
template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<NumericalTag> constructor);

}
