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

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/state.hpp"

namespace mimir::dl
{

/**
 * RefinementStateListPruningFunction
 */

RefinementStateListPruningFunction::RefinementStateListPruningFunction(const PDDLFactories& pddl_factories, Problem problem, StateList states) :
    RefinementPruningFunction(),
    m_pddl_factories(pddl_factories),
    m_problem(problem),
    m_states(std::move(states)),
    m_concept_denotation_builder(),
    m_role_denotation_builder(),
    m_concept_denotation_repository(),
    m_role_denotation_repository()

{
    m_role_denotation_builder.get_data().resize(m_problem->get_objects().size());
}

bool RefinementStateListPruningFunction::should_prune(Constructor<Concept> concept_) { return should_prune_impl(concept_); }

bool RefinementStateListPruningFunction::should_prune(Constructor<Role> role_) { return should_prune_impl(role_); }

template<IsConceptOrRole D>
bool RefinementStateListPruningFunction::should_prune_impl(Constructor<D> constructor)
{
    auto denotations = DenotationsList();
    denotations.reserve(m_states.size());

    for (const auto& state : m_states)
    {
        auto evaluation_context = EvaluationContext(m_pddl_factories,
                                                    m_problem,
                                                    state,
                                                    m_concept_denotation_builder,
                                                    m_role_denotation_builder,
                                                    m_concept_denotation_repository,
                                                    m_role_denotation_repository);

        const auto eval = constructor->evaluate(evaluation_context);

        denotations.push_back(reinterpret_cast<uintptr_t>(eval));
    }

    return !m_denotations_repository.insert(denotations).second;
}

template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<Concept> constructor);
template bool RefinementStateListPruningFunction::should_prune_impl(Constructor<Role> constructor);

RefinementBrfsResult refine_brfs(Problem problem,
                                 const grammar::Grammar grammar,
                                 const RefinementBrfsOptions& options,
                                 VariadicConstructorFactory& ref_constructor_repos,
                                 RefinementPruningFunction& ref_pruning_function)
{
    auto result = RefinementBrfsResult();

    // TODO: implement generator logic
    auto concepts_by_complexity = std::vector<ConstructorList<Concept>>();
    auto roles_by_complexity = std::vector<ConstructorList<Role>>();

    if (options.max_complexity > 0)
    {
        // Sample code on how to generate concept atomic state
        for (const auto& predicate : problem->get_domain()->get_predicates<Static>())
        {
            if (predicate->get_arity() != 1)
                continue;

            const auto concept_ = ref_constructor_repos.get<ConceptAtomicStateFactory<Static>>().get_or_create<ConceptAtomicStateImpl<Static>>(predicate);

            if (grammar.test_match(concept_) && !ref_pruning_function.should_prune(concept_))
            {
                if (concepts_by_complexity.size() < 1)
                    concepts_by_complexity.resize(1);

                concepts_by_complexity[0].push_back(concept_);
            }
        }

        // Sample code on how to generate concept intersection
        const auto cur_complexity = 2;

        for (size_t i = 1; i < cur_complexity; ++i)
        {
            const auto j = cur_complexity - i;

            for (const auto& concept_left : concepts_by_complexity.at(i))
            {
                for (const auto& concept_right : concepts_by_complexity.at(j))
                {
                    const auto concept_ =
                        ref_constructor_repos.get<ConceptIntersectionFactory>().get_or_create<ConceptIntersectionImpl>(concept_left, concept_right);

                    if (grammar.test_match(concept_) && !ref_pruning_function.should_prune(concept_))
                    {
                        if (concepts_by_complexity.size() < cur_complexity)
                            concepts_by_complexity.resize(cur_complexity + 1);

                        concepts_by_complexity[cur_complexity].push_back(concept_);
                    }
                }
            }
        }
    }

    return result;
}

}
