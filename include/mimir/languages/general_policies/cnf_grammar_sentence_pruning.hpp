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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_CNF_GRAMMAR_SENTENCE_PRUNING_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_CNF_GRAMMAR_SENTENCE_PRUNING_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/generalized_state_space/class_graph.hpp"
#include "mimir/languages/description_logics/cnf_grammar_sentence_pruning.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>

namespace mimir::languages::general_policies
{

/// @brief `GeneralPoliciesRefinementPruningFunction` implements a pruning function based on a given state list.
/// A feature is pruned if it does not evaluate differently on at least one state compared to a previously tested feature.
/// This ensures that only features with unique evaluations across all states are retained.
class GeneralPoliciesRefinementPruningFunction : public dl::IRefinementPruningFunction
{
public:
    GeneralPoliciesRefinementPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space,
                                             dl::DenotationRepositories& ref_denotation_repositories);

    GeneralPoliciesRefinementPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space,
                                             const graphs::ClassGraph& class_graph,
                                             dl::DenotationRepositories& ref_denotation_repositories);

    GeneralPoliciesRefinementPruningFunction(search::StateProblemList states,
                                             search::StateProblemPairList transitions,
                                             dl::DenotationRepositories& ref_denotation_repositories);

    /// @brief Tests whether a concept should be pruned.
    /// @param concept_ The concept to evaluate.
    /// @return True if the concept is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(dl::Constructor<dl::ConceptTag> concept_) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(dl::Constructor<dl::RoleTag> role) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(dl::Constructor<dl::BooleanTag> boolean) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(dl::Constructor<dl::NumericalTag> numerical) override;

    /**
     * Getters
     */

    const search::StateProblemPairList& get_transitions() const;

private:
    template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
    bool should_prune_impl(dl::Constructor<D> constructor);

    dl::StateListRefinementPruningFunction m_state_list_pruning_function;

    search::StateProblemPairList m_transitions;

    dl::DenotationRepositories& m_denotation_repositories;

    enum class AbstractFeature
    {
        POSITIVE_BOOLEAN_CONDITION,
        NEGATIVE_BOOLEAN_CONDITION,
        GREATER_NUMERICAL_CONDITION,
        EQUAL_NUMERICAL_CONDITION,
        POSITIVE_BOOLEAN_EFFECT,
        NEGATIVE_BOOLEAN_EFFECT,
        UNCHANGED_BOOLEAN_EFFECT,
        INCREASE_NUMERICAL_EFFECT,
        DECREASE_NUMERICAL_EFFECT,
        UNCHANGED_NUMERICAL_EFFECT
    };

    using AbstractFeatureList = std::vector<AbstractFeature>;

    UnorderedSet<AbstractFeatureList> m_abstract_features;
};

}

#endif
