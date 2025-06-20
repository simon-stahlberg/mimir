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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_SENTENCE_PRUNING_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_SENTENCE_PRUNING_HPP_

#include "mimir/datasets/declarations.hpp"
#include "mimir/datasets/generalized_state_space/class_graph.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>

namespace mimir::languages::dl
{

/// @brief `IRefinementPruningFunction` defines an interface for pruning dl constructors (= features).
class IRefinementPruningFunction
{
public:
    virtual ~IRefinementPruningFunction() = default;

    /// @brief Tests whether the given concept should be pruned.
    /// @param concept_ is the concept to be tested
    /// @return true iff the concept must be pruned, false otherwise.
    virtual bool should_prune(Constructor<ConceptTag> concept_) = 0;

    /// @brief Tests whether the given role should be pruned.
    /// @param role_ is the role to be tested
    /// @return true iff the role must be pruned, false otherwise.
    virtual bool should_prune(Constructor<RoleTag> role) = 0;

    /// @brief Tests whether the given role should be pruned.
    /// @param role_ is the role to be tested
    /// @return true iff the role must be pruned, false otherwise.
    virtual bool should_prune(Constructor<BooleanTag> boolean) = 0;

    /// @brief Tests whether the given role should be pruned.
    /// @param role_ is the role to be tested
    /// @return true iff the role must be pruned, false otherwise.
    virtual bool should_prune(Constructor<NumericalTag> numerical) = 0;
};

/// @brief `StateListRefinementPruningFunction` implements a pruning function based on a given state list.
/// A feature is pruned if it does not evaluate differently on at least one state compared to a previously tested feature.
/// This ensures that only features with unique evaluations across all states are retained.
class StateListRefinementPruningFunction : public IRefinementPruningFunction
{
public:
    StateListRefinementPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space, DenotationRepositories& ref_denotation_repositories);

    StateListRefinementPruningFunction(const datasets::GeneralizedStateSpace& generalized_state_space,
                                       const graphs::ClassGraph& class_graph,
                                       DenotationRepositories& ref_denotation_repositories);

    StateListRefinementPruningFunction(search::StateProblemList states, DenotationRepositories& ref_denotation_repositories);

    /// @brief Tests whether a concept should be pruned.
    /// @param concept_ The concept to evaluate.
    /// @return True if the concept is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<ConceptTag> concept_) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<RoleTag> role) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<BooleanTag> boolean) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<NumericalTag> numerical) override;

    /**
     * Getters
     */

    const search::StateProblemList& get_states() const;

private:
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    bool should_prune_impl(Constructor<D> constructor);

    DenotationRepositories& m_denotation_repositories;

    DenotationListSets m_denotations_repositories;

    search::StateProblemList m_states;
};

}

#endif
