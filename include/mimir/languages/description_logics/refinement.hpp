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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_REFINEMENT_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_REFINEMENT_HPP_

#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/equal_to.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/hash.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>

namespace mimir::dl
{

/// @brief `RefinementPruningFunction` defines an interface for pruning dl constructors (= features).
class RefinementPruningFunction
{
public:
    virtual ~RefinementPruningFunction() = default;

    /// @brief Tests whether the given concept should be pruned.
    /// @param concept_ is the concept to be tested
    /// @return true iff the concept must be pruned, false otherwise.
    virtual bool should_prune(Constructor<Concept> concept_) = 0;

    /// @brief Tests whether the given role should be pruned.
    /// @param role_ is the role to be tested
    /// @return true iff the role must be pruned, false otherwise.
    virtual bool should_prune(Constructor<Role> role_) = 0;
};

/// @brief `RefinementStateListPruningFunction` implements a pruning function based on a given state list.
/// A feature is pruned if it does not evaluate differently on at least one state compared to a previously tested feature.
/// This ensures that only features with unique evaluations across all states are retained.
class RefinementStateListPruningFunction : public RefinementPruningFunction
{
public:
    RefinementStateListPruningFunction(const PDDLRepositories& pddl_repositories, Problem problem, StateList states);

    /// @brief Tests whether a concept should be pruned.
    /// @param concept_ The concept to evaluate.
    /// @return True if the concept is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<Concept> concept_) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<Role> role_) override;

private:
    template<ConstructorTag D>
    bool should_prune_impl(Constructor<D> constructor);

    const PDDLRepositories& m_pddl_repositories;  ///< The pddl factories.
    Problem m_problem;                            ///< The problem definition used for evaluating features.
    StateList m_states;                           ///< The list of states used for evaluating features and pruning.

    ConstructorTagToDenotationType m_denotation_builder;

    ConstructorTagToDenotationRepository m_denotation_repository;

    /// @brief Uniquely store feature denotations among all states.
    /// Each state feature denotation is uniquely identified by its memory address.
    /// These addresses are stored as vectors of `uintptr_t`.
    /// Two identical vectors imply identical evaluations across all states.
    using DenotationsList = std::vector<uintptr_t>;
    std::unordered_set<DenotationsList, Hash<DenotationsList>> m_denotations_repository;
};

/**
 * Brfs-style refinement.
 *
 * Generates concepts and roles with increasing complexity starting at 1.
 */

namespace refinement_brfs
{

struct Options
{
    size_t verbosity = 0;  ///< 0=quiet, 1=normal, 2=debug

    size_t max_complexity = 0;
    size_t max_memory_usage_in_kb = 0;
    size_t max_execution_time_in_ms = 0;

    using ConstructorTagToSizeT =
        boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::size_t>, boost::hana::pair<boost::hana::type<Role>, std::size_t>>;

    ConstructorTagToSizeT max_constructors = ConstructorTagToSizeT();
};

struct Statistics
{
    size_t memory_usage_in_kb = 0;
    size_t execution_time_ms = 0;

    using ConstructorTagToSizeT =
        boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::size_t>, boost::hana::pair<boost::hana::type<Role>, std::size_t>>;

    ConstructorTagToSizeT num_generated = ConstructorTagToSizeT();
    ConstructorTagToSizeT num_pruned = ConstructorTagToSizeT();
    ConstructorTagToSizeT num_rejected_by_grammar = ConstructorTagToSizeT();
};

struct Result
{
    using ConstructorTagToConstructorList = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, ConstructorList<Concept>>,
                                                             boost::hana::pair<boost::hana::type<Role>, ConstructorList<Role>>>;

    ConstructorTagToConstructorList constructors = ConstructorTagToConstructorList();

    Statistics statistics = Statistics();
};

extern Result refine(Problem problem,
                     const grammar::Grammar& grammar,
                     const Options& options,
                     ConstructorTagToRepository& ref_constructor_repositories,
                     RefinementPruningFunction& ref_pruning_function);

}

}

#endif
