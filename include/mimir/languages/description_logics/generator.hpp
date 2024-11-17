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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GENERATOR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GENERATOR_HPP_

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/equal_to.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/hash.hpp"

#include <loki/loki.hpp>

namespace mimir::dl
{

using ConceptBotFactory = loki::UniqueFactory<ConceptBotImpl, UniqueDLHasher<const ConceptBotImpl*>, UniqueDLEqualTo<const ConceptBotImpl*>>;
using ConceptTopFactory = loki::UniqueFactory<ConceptTopImpl, UniqueDLHasher<const ConceptTopImpl*>, UniqueDLEqualTo<const ConceptTopImpl*>>;
template<PredicateCategory P>
using ConceptAtomicStateFactory =
    loki::UniqueFactory<ConceptAtomicStateImpl<P>, UniqueDLHasher<const ConceptAtomicStateImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using ConceptAtomicGoalFactory =
    loki::UniqueFactory<ConceptAtomicGoalImpl<P>, UniqueDLHasher<const ConceptAtomicGoalImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicGoalImpl<P>*>>;
using ConceptIntersectionFactory =
    loki::UniqueFactory<ConceptIntersectionImpl, UniqueDLHasher<const ConceptIntersectionImpl*>, UniqueDLEqualTo<const ConceptIntersectionImpl*>>;
using ConceptUnionFactory = loki::UniqueFactory<ConceptUnionImpl, UniqueDLHasher<const ConceptUnionImpl*>, UniqueDLEqualTo<const ConceptUnionImpl*>>;
using ConceptNegationFactory =
    loki::UniqueFactory<ConceptNegationImpl, UniqueDLHasher<const ConceptNegationImpl*>, UniqueDLEqualTo<const ConceptNegationImpl*>>;
using ConceptValueRestrictionFactory =
    loki::UniqueFactory<ConceptValueRestrictionImpl, UniqueDLHasher<const ConceptValueRestrictionImpl*>, UniqueDLEqualTo<const ConceptValueRestrictionImpl*>>;
using ConceptExistentialQuantificationFactory = loki::UniqueFactory<ConceptExistentialQuantificationImpl,
                                                                    UniqueDLHasher<const ConceptExistentialQuantificationImpl*>,
                                                                    UniqueDLEqualTo<const ConceptExistentialQuantificationImpl*>>;
using ConceptRoleValueMapContainmentFactory = loki::UniqueFactory<ConceptRoleValueMapContainmentImpl,
                                                                  UniqueDLHasher<const ConceptRoleValueMapContainmentImpl*>,
                                                                  UniqueDLEqualTo<const ConceptRoleValueMapContainmentImpl*>>;
using ConceptRoleValueMapEqualityFactory = loki::UniqueFactory<ConceptRoleValueMapEqualityImpl,
                                                               UniqueDLHasher<const ConceptRoleValueMapEqualityImpl*>,
                                                               UniqueDLEqualTo<const ConceptRoleValueMapEqualityImpl*>>;
using ConceptNominalFactory = loki::UniqueFactory<ConceptNominalImpl, UniqueDLHasher<const ConceptNominalImpl*>, UniqueDLEqualTo<const ConceptNominalImpl*>>;

using RoleUniversalFactory = loki::UniqueFactory<RoleUniversalImpl, UniqueDLHasher<const RoleUniversalImpl*>, UniqueDLEqualTo<const RoleUniversalImpl*>>;
template<PredicateCategory P>
using RolePredicateStateFactory =
    loki::UniqueFactory<RoleAtomicStateImpl<P>, UniqueDLHasher<const RoleAtomicStateImpl<P>*>, UniqueDLEqualTo<const RoleAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using RolePredicateGoalFactory =
    loki::UniqueFactory<RoleAtomicGoalImpl<P>, UniqueDLHasher<const RoleAtomicGoalImpl<P>*>, UniqueDLEqualTo<const RoleAtomicGoalImpl<P>*>>;
using RoleIntersectionFactory =
    loki::UniqueFactory<RoleIntersectionImpl, UniqueDLHasher<const RoleIntersectionImpl*>, UniqueDLEqualTo<const RoleIntersectionImpl*>>;
using RoleUnionFactory = loki::UniqueFactory<RoleUnionImpl, UniqueDLHasher<const RoleUnionImpl*>, UniqueDLEqualTo<const RoleUnionImpl*>>;
using RoleComplementFactory = loki::UniqueFactory<RoleComplementImpl, UniqueDLHasher<const RoleComplementImpl*>, UniqueDLEqualTo<const RoleComplementImpl*>>;
using RoleInverseFactory = loki::UniqueFactory<RoleInverseImpl, UniqueDLHasher<const RoleInverseImpl*>, UniqueDLEqualTo<const RoleInverseImpl*>>;
using RoleCompositionFactory =
    loki::UniqueFactory<RoleCompositionImpl, UniqueDLHasher<const RoleCompositionImpl*>, UniqueDLEqualTo<const RoleCompositionImpl*>>;
using RoleTransitiveClosureFactory =
    loki::UniqueFactory<RoleTransitiveClosureImpl, UniqueDLHasher<const RoleTransitiveClosureImpl*>, UniqueDLEqualTo<const RoleTransitiveClosureImpl*>>;
using RoleReflexiveTransitiveClosureFactory = loki::UniqueFactory<RoleReflexiveTransitiveClosureImpl,
                                                                  UniqueDLHasher<const RoleReflexiveTransitiveClosureImpl*>,
                                                                  UniqueDLEqualTo<const RoleReflexiveTransitiveClosureImpl*>>;
using RoleRestrictionFactory =
    loki::UniqueFactory<RoleRestrictionImpl, UniqueDLHasher<const RoleRestrictionImpl*>, UniqueDLEqualTo<const RoleRestrictionImpl*>>;
using RoleIdentityFactory = loki::UniqueFactory<RoleIdentityImpl, UniqueDLHasher<const RoleIdentityImpl*>, UniqueDLEqualTo<const RoleIdentityImpl*>>;

using VariadicConstructorFactory = loki::VariadicContainer<ConceptBotFactory,
                                                           ConceptTopFactory,
                                                           ConceptAtomicStateFactory<Static>,
                                                           ConceptAtomicStateFactory<Fluent>,
                                                           ConceptAtomicStateFactory<Derived>,
                                                           ConceptAtomicGoalFactory<Static>,
                                                           ConceptAtomicGoalFactory<Fluent>,
                                                           ConceptAtomicGoalFactory<Derived>,
                                                           ConceptIntersectionFactory,
                                                           ConceptUnionFactory,
                                                           ConceptNegationFactory,
                                                           ConceptValueRestrictionFactory,
                                                           ConceptExistentialQuantificationFactory,
                                                           ConceptRoleValueMapContainmentFactory,
                                                           ConceptRoleValueMapEqualityFactory,
                                                           ConceptNominalFactory,
                                                           RoleUniversalFactory,
                                                           RolePredicateStateFactory<Static>,
                                                           RolePredicateStateFactory<Fluent>,
                                                           RolePredicateStateFactory<Derived>,
                                                           RolePredicateGoalFactory<Static>,
                                                           RolePredicateGoalFactory<Fluent>,
                                                           RolePredicateGoalFactory<Derived>,
                                                           RoleIntersectionFactory,
                                                           RoleUnionFactory,
                                                           RoleComplementFactory,
                                                           RoleInverseFactory,
                                                           RoleCompositionFactory,
                                                           RoleTransitiveClosureFactory,
                                                           RoleReflexiveTransitiveClosureFactory,
                                                           RoleRestrictionFactory,
                                                           RoleIdentityFactory>;

extern VariadicConstructorFactory create_default_variadic_constructor_factory();

struct GeneratorOptions
{
    size_t max_memory_in_kb;
    size_t max_time_in_ms;
};

/// @brief `GeneratorPruningFunction` defines an interface for pruning dl constructors (= features).
class GeneratorPruningFunction
{
public:
    virtual ~GeneratorPruningFunction() = default;

    /// @brief Tests whether the given concept should be pruned.
    /// @param concept_ is the concept to be tested
    /// @return true iff the concept must be pruned, false otherwise.
    virtual bool should_prune(Constructor<Concept> concept_) = 0;

    /// @brief Tests whether the given role should be pruned.
    /// @param role_ is the role to be tested
    /// @return true iff the role must be pruned, false otherwise.
    virtual bool should_prune(Constructor<Role> role_) = 0;
};

/// @brief `GeneratorStateListPruningFunction` implements a pruning function based on a given state list.
/// A feature is pruned if it does not evaluate differently on at least one state compared to a previously tested feature.
/// This ensures that only features with unique evaluations across all states are retained.
class GeneratorStateListPruningFunction : public GeneratorPruningFunction
{
public:
    GeneratorStateListPruningFunction(const PDDLFactories& pddl_factories, Problem problem, StateList states);

    /// @brief Tests whether a concept should be pruned.
    /// @param concept_ The concept to evaluate.
    /// @return True if the concept is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<Concept> concept_) override;

    /// @brief Tests whether a role should be pruned.
    /// @param role_ The role to evaluate.
    /// @return True if the role is pruned (i.e., its evaluation is not unique across states), false otherwise.
    bool should_prune(Constructor<Role> role_) override;

private:
    template<IsConceptOrRole D>
    bool should_prune_impl(Constructor<D> constructor);

    const PDDLFactories& m_pddl_factories;                 ///< The pddl factories.
    Problem m_problem;                                     ///< The problem definition used for evaluating features.
    StateList m_states;                                    ///< The list of states used for evaluating features and pruning.
    DenotationImpl<Concept> m_concept_denotation_builder;  ///< Temporary denotation used during evaluation
    DenotationImpl<Role> m_role_denotation_builder;        ///< Temporary denotation used during evaluation

    /// @brief Repository for managing concept denotations.
    /// This stores the computed denotations for each concept feature across all states.
    DenotationRepository<Concept> m_concept_denotation_repository;

    /// @brief Repository for managing role denotations.
    /// This stores the computed denotations for each role feature across all states.
    DenotationRepository<Role> m_role_denotation_repository;

    /// @brief Uniquely store feature denotations among all states.
    /// Each state feature denotation is uniquely identified by its memory address.
    /// These addresses are stored as vectors of `uintptr_t`.
    /// Two identical vectors imply identical evaluations across all states.
    using DenotationsList = std::vector<uintptr_t>;
    std::unordered_set<DenotationsList, Hash<DenotationsList>> m_denotations_repository;
};

extern std::tuple<ConstructorList<Concept>, ConstructorList<Role>> generate(const grammar::Grammar grammar,
                                                                            const GeneratorOptions& options,
                                                                            VariadicConstructorFactory& ref_constructor_repos,
                                                                            GeneratorPruningFunction& ref_pruning_function);

}

#endif
