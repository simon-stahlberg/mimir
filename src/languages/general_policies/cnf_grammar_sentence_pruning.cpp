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

#include "mimir/languages/general_policies/cnf_grammar_sentence_pruning.hpp"

#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/description_logics/grammar.hpp"

namespace mimir::languages::general_policies
{

template<>
bool GeneralPoliciesRefinementPruningFunction::should_prune_impl<dl::BooleanTag>(dl::Constructor<dl::BooleanTag> constructor)
{
    auto denotations = AbstractFeatureList {};

    auto src_eval_context = dl::EvaluationContext(nullptr, nullptr, m_denotation_repositories);
    auto dst_eval_context = dl::EvaluationContext(nullptr, nullptr, m_denotation_repositories);

    for (const auto& [src, dst] : m_transitions)
    {
        const auto& [src_state, src_problem] = src;
        const auto& [dst_state, dst_problem] = dst;

        src_eval_context.set_state(src_state);
        src_eval_context.set_problem(src_problem);
        const auto src_eval = constructor->evaluate(src_eval_context)->get_data();

        dst_eval_context.set_state(dst_state);
        dst_eval_context.set_problem(dst_problem);
        const auto dst_eval = constructor->evaluate(dst_eval_context)->get_data();

        // Handle condition
        if (src_eval)
        {
            denotations.push_back(AbstractFeature::POSITIVE_BOOLEAN_CONDITION);
        }
        else
        {
            denotations.push_back(AbstractFeature::NEGATIVE_BOOLEAN_CONDITION);
        }

        // Handle effect
        if (src_eval == dst_eval)
        {
            denotations.push_back(AbstractFeature::UNCHANGED_BOOLEAN_EFFECT);
        }
        else if (!src_eval && dst_eval)
        {
            denotations.push_back(AbstractFeature::POSITIVE_BOOLEAN_EFFECT);
        }
        else if (src_eval && !dst_eval)
        {
            denotations.push_back(AbstractFeature::NEGATIVE_BOOLEAN_EFFECT);
        }
        else
        {
            throw std::logic_error("Unexpeded case reached.");
        }
    }

    const auto inserted = m_abstract_features.insert(denotations).second;

    return !inserted;
}

template<>
bool GeneralPoliciesRefinementPruningFunction::should_prune_impl<dl::NumericalTag>(dl::Constructor<dl::NumericalTag> constructor)
{
    auto denotations = AbstractFeatureList {};

    auto src_eval_context = dl::EvaluationContext(nullptr, nullptr, m_denotation_repositories);
    auto dst_eval_context = dl::EvaluationContext(nullptr, nullptr, m_denotation_repositories);

    for (const auto& [src, dst] : m_transitions)
    {
        const auto& [src_state, src_problem] = src;
        const auto& [dst_state, dst_problem] = dst;

        src_eval_context.set_state(src_state);
        src_eval_context.set_problem(src_problem);
        const auto src_eval = constructor->evaluate(src_eval_context)->get_data();

        dst_eval_context.set_state(dst_state);
        dst_eval_context.set_problem(dst_problem);
        const auto dst_eval = constructor->evaluate(dst_eval_context)->get_data();

        // Handle condition
        if (src_eval > 0)
        {
            denotations.push_back(AbstractFeature::GREATER_NUMERICAL_CONDITION);
        }
        else
        {
            denotations.push_back(AbstractFeature::EQUAL_NUMERICAL_CONDITION);
        }

        // Handle effect
        if (src_eval == dst_eval)
        {
            denotations.push_back(AbstractFeature::UNCHANGED_NUMERICAL_EFFECT);
        }
        else if (src_eval > dst_eval)
        {
            denotations.push_back(AbstractFeature::DECREASE_NUMERICAL_EFFECT);
        }
        else if (src_eval < dst_eval)
        {
            denotations.push_back(AbstractFeature::INCREASE_NUMERICAL_EFFECT);
        }
        else
        {
            throw std::logic_error("Unexpeded case reached.");
        }
    }

    const auto inserted = m_abstract_features.insert(denotations).second;

    return !inserted;
}

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
bool GeneralPoliciesRefinementPruningFunction::should_prune_impl(dl::Constructor<D> constructor)
{
    return m_state_list_pruning_function.should_prune(constructor);
}

template bool GeneralPoliciesRefinementPruningFunction::should_prune_impl(dl::Constructor<dl::ConceptTag> constructor);
template bool GeneralPoliciesRefinementPruningFunction::should_prune_impl(dl::Constructor<dl::RoleTag> constructor);

GeneralPoliciesRefinementPruningFunction::GeneralPoliciesRefinementPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space,
                                                                                   dl::DenotationRepositories& ref_denotation_repositories) :
    GeneralPoliciesRefinementPruningFunction(generalized_state_space, generalized_state_space->get_graph(), ref_denotation_repositories)
{
}

GeneralPoliciesRefinementPruningFunction::GeneralPoliciesRefinementPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space,
                                                                                   const graphs::ClassGraph& class_graph,
                                                                                   dl::DenotationRepositories& ref_denotation_repositories) :
    m_state_list_pruning_function(dl::StateListRefinementPruningFunction(generalized_state_space, class_graph, ref_denotation_repositories)),
    m_transitions(),
    m_denotation_repositories(ref_denotation_repositories)
{
    for (const auto& edge : class_graph.get_edges())
    {
        const auto& src_problem_v = generalized_state_space->get_problem_vertex(generalized_state_space->get_graph().get_vertex(edge.get_source()));
        const auto& dst_problem_v = generalized_state_space->get_problem_vertex(generalized_state_space->get_graph().get_vertex(edge.get_target()));
        const auto& src_state = graphs::get_state(src_problem_v);
        const auto& src_problem = graphs::get_problem(src_problem_v);
        const auto& dst_state = graphs::get_state(dst_problem_v);
        const auto& dst_problem = graphs::get_problem(dst_problem_v);

        m_transitions.emplace_back(std::make_pair(src_state, src_problem), std::make_pair(dst_state, dst_problem));
    }
    // Sort to decrease allocations/deallocations during sequential evaluation.
    std::sort(m_transitions.begin(),
              m_transitions.end(),
              [](auto&& lhs, auto&& rhs) { return lhs.first.second->get_objects().size() > rhs.first.second->get_objects().size(); });
}

GeneralPoliciesRefinementPruningFunction::GeneralPoliciesRefinementPruningFunction(search::StateProblemList states,
                                                                                   search::StateProblemPairList transitions,
                                                                                   dl::DenotationRepositories& ref_denotation_repositories) :
    m_state_list_pruning_function(dl::StateListRefinementPruningFunction(std::move(states), ref_denotation_repositories)),
    m_transitions(std::move(transitions)),
    m_denotation_repositories(ref_denotation_repositories)
{
    // Sort to decrease allocations/deallocations during sequential evaluation.
    std::sort(m_transitions.begin(),
              m_transitions.end(),
              [](auto&& lhs, auto&& rhs) { return lhs.first.second->get_objects().size() > rhs.first.second->get_objects().size(); });
}

bool GeneralPoliciesRefinementPruningFunction::should_prune(dl::Constructor<dl::ConceptTag> concept_) { return should_prune_impl(concept_); }

bool GeneralPoliciesRefinementPruningFunction::should_prune(dl::Constructor<dl::RoleTag> role) { return should_prune_impl(role); }

bool GeneralPoliciesRefinementPruningFunction::should_prune(dl::Constructor<dl::BooleanTag> boolean) { return should_prune_impl(boolean); }

bool GeneralPoliciesRefinementPruningFunction::should_prune(dl::Constructor<dl::NumericalTag> numerical) { return should_prune_impl(numerical); }

const search::StateProblemPairList& GeneralPoliciesRefinementPruningFunction::get_transitions() const { return m_transitions; }

}
