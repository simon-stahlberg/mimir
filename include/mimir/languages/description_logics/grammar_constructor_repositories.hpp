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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTOR_REPOSITORIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTOR_REPOSITORIES_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

#include <boost/hana.hpp>
#include <loki/loki.hpp>

namespace mimir::dl::grammar
{

/**
 * Grammar
 */

template<typename T>
using SegmentedDLRepository = loki::SegmentedRepository<T>;

template<ConceptOrRole D>
using NonTerminalFactory = SegmentedDLRepository<NonTerminalImpl<D>>;

template<ConceptOrRole D>
using ChoiceFactory = SegmentedDLRepository<ConstructorOrNonTerminalImpl<D>>;

template<ConceptOrRole D>
using DerivationRuleFactory = SegmentedDLRepository<DerivationRuleImpl<D>>;

using ConceptBotRepository = SegmentedDLRepository<ConceptBotImpl>;
using ConceptTopRepository = SegmentedDLRepository<ConceptTopImpl>;
template<StaticOrFluentOrDerived P>
using ConceptAtomicStateRepository = SegmentedDLRepository<ConceptAtomicStateImpl<P>>;
template<StaticOrFluentOrDerived P>
using ConceptAtomicGoalRepository = SegmentedDLRepository<ConceptAtomicGoalImpl<P>>;
using ConceptIntersectionRepository = SegmentedDLRepository<ConceptIntersectionImpl>;
using ConceptUnionRepository = SegmentedDLRepository<ConceptUnionImpl>;
using ConceptNegationRepository = SegmentedDLRepository<ConceptNegationImpl>;
using ConceptValueRestrictionRepository = SegmentedDLRepository<ConceptValueRestrictionImpl>;
using ConceptExistentialQuantificationRepository = SegmentedDLRepository<ConceptExistentialQuantificationImpl>;
using ConceptRoleValueMapContainmentRepository = SegmentedDLRepository<ConceptRoleValueMapContainmentImpl>;
using ConceptRoleValueMapEqualityRepository = SegmentedDLRepository<ConceptRoleValueMapEqualityImpl>;
using ConceptNominalRepository = SegmentedDLRepository<ConceptNominalImpl>;

using RoleUniversalRepository = SegmentedDLRepository<RoleUniversalImpl>;
template<StaticOrFluentOrDerived P>
using RoleAtomicStateRepository = SegmentedDLRepository<RoleAtomicStateImpl<P>>;
template<StaticOrFluentOrDerived P>
using RoleAtomicGoalRepository = SegmentedDLRepository<RoleAtomicGoalImpl<P>>;
using RoleIntersectionRepository = SegmentedDLRepository<RoleIntersectionImpl>;
using RoleUnionRepository = SegmentedDLRepository<RoleUnionImpl>;
using RoleComplementRepository = SegmentedDLRepository<RoleComplementImpl>;
using RoleInverseRepository = SegmentedDLRepository<RoleInverseImpl>;
using RoleCompositionRepository = SegmentedDLRepository<RoleCompositionImpl>;
using RoleTransitiveClosureRepository = SegmentedDLRepository<RoleTransitiveClosureImpl>;
using RoleReflexiveTransitiveClosureRepository = SegmentedDLRepository<RoleReflexiveTransitiveClosureImpl>;
using RoleRestrictionRepository = SegmentedDLRepository<RoleRestrictionImpl>;
using RoleIdentityFactory = SegmentedDLRepository<RoleIdentityImpl>;

using HanaConstructorRepositories =
    boost::hana::map<boost::hana::pair<boost::hana::type<NonTerminalImpl<Concept>>, NonTerminalFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<Concept>>, ChoiceFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<DerivationRuleImpl<Concept>>, DerivationRuleFactory<Concept>>,
                     boost::hana::pair<boost::hana::type<ConceptBotImpl>, ConceptBotRepository>,
                     boost::hana::pair<boost::hana::type<ConceptTopImpl>, ConceptTopRepository>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Static>>, ConceptAtomicStateRepository<Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Fluent>>, ConceptAtomicStateRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<Derived>>, ConceptAtomicStateRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Static>>, ConceptAtomicGoalRepository<Static>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Fluent>>, ConceptAtomicGoalRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<Derived>>, ConceptAtomicGoalRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<ConceptIntersectionImpl>, ConceptIntersectionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptUnionImpl>, ConceptUnionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptNegationImpl>, ConceptNegationRepository>,
                     boost::hana::pair<boost::hana::type<ConceptValueRestrictionImpl>, ConceptValueRestrictionRepository>,
                     boost::hana::pair<boost::hana::type<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationRepository>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentRepository>,
                     boost::hana::pair<boost::hana::type<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityRepository>,
                     boost::hana::pair<boost::hana::type<ConceptNominalImpl>, ConceptNominalRepository>,
                     boost::hana::pair<boost::hana::type<NonTerminalImpl<Role>>, NonTerminalFactory<Role>>,
                     boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<Role>>, ChoiceFactory<Role>>,
                     boost::hana::pair<boost::hana::type<DerivationRuleImpl<Role>>, DerivationRuleFactory<Role>>,
                     boost::hana::pair<boost::hana::type<RoleUniversalImpl>, RoleUniversalRepository>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Static>>, RoleAtomicStateRepository<Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Fluent>>, RoleAtomicStateRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<Derived>>, RoleAtomicStateRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Static>>, RoleAtomicGoalRepository<Static>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Fluent>>, RoleAtomicGoalRepository<Fluent>>,
                     boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<Derived>>, RoleAtomicGoalRepository<Derived>>,
                     boost::hana::pair<boost::hana::type<RoleIntersectionImpl>, RoleIntersectionRepository>,
                     boost::hana::pair<boost::hana::type<RoleUnionImpl>, RoleUnionRepository>,
                     boost::hana::pair<boost::hana::type<RoleComplementImpl>, RoleComplementRepository>,
                     boost::hana::pair<boost::hana::type<RoleInverseImpl>, RoleInverseRepository>,
                     boost::hana::pair<boost::hana::type<RoleCompositionImpl>, RoleCompositionRepository>,
                     boost::hana::pair<boost::hana::type<RoleTransitiveClosureImpl>, RoleTransitiveClosureRepository>,
                     boost::hana::pair<boost::hana::type<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureRepository>,
                     boost::hana::pair<boost::hana::type<RoleRestrictionImpl>, RoleRestrictionRepository>,
                     boost::hana::pair<boost::hana::type<RoleIdentityImpl>, RoleIdentityFactory>>;

class ConstructorRepositories
{
private:
    HanaConstructorRepositories m_repositories;

public:
    ConstructorRepositories() = default;
    ConstructorRepositories(const ConstructorRepositories& other) = delete;
    ConstructorRepositories& operator=(const ConstructorRepositories& other) = delete;
    ConstructorRepositories(ConstructorRepositories&& other) = default;
    ConstructorRepositories& operator=(ConstructorRepositories&& other) = default;

    HanaConstructorRepositories& get_repositories();
    const HanaConstructorRepositories& get_repositories() const;

    template<ConceptOrRole D>
    DerivationRule<D> get_or_create_derivation_rule(NonTerminal<D> non_terminal, ConstructorOrNonTerminalList<D> constructor_or_non_terminals);
    template<ConceptOrRole D>
    NonTerminal<D> get_or_create_nonterminal(std::string name);
    template<ConceptOrRole D>
    ConstructorOrNonTerminal<D> get_or_create_constructor_or_nonterminal(std::variant<Constructor<D>, NonTerminal<D>> choice);

    /* Concepts */
    ConceptBot get_or_create_concept_bot();
    ConceptTop get_or_create_concept_top();
    template<StaticOrFluentOrDerived P>
    ConceptAtomicState<P> get_or_create_concept_atomic_state(Predicate<P> predicate);
    template<StaticOrFluentOrDerived P>
    ConceptAtomicGoal<P> get_or_create_concept_atomic_goal(Predicate<P> predicate, bool is_negated);
    ConceptIntersection get_or_create_concept_intersection(ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                                                           ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right);
    ConceptUnion get_or_create_concept_union(ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                                             ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right);
    ConceptNegation get_or_create_concept_negation(ConstructorOrNonTerminal<Concept> concept_or_non_terminal);
    ConceptValueRestriction get_or_create_concept_value_restriction(ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                                    ConstructorOrNonTerminal<Concept> concept_or_non_terminal);
    ConceptExistentialQuantification get_or_create_concept_existential_quantification(ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                                                      ConstructorOrNonTerminal<Concept> concept_or_non_terminal);
    ConceptRoleValueMapContainment get_or_create_concept_role_value_map_containment(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                                    ConstructorOrNonTerminal<Role> role_or_non_terminal_right);
    ConceptRoleValueMapEquality get_or_create_concept_role_value_map_equality(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                                              ConstructorOrNonTerminal<Role> role_or_non_terminal_right);
    ConceptNominal get_or_create_concept_nominal(Object object);

    /* Roles */
    RoleUniversal get_or_create_role_universal();
    template<StaticOrFluentOrDerived P>
    RoleAtomicState<P> get_or_create_role_atomic_state(Predicate<P> predicate);
    template<StaticOrFluentOrDerived P>
    RoleAtomicGoal<P> get_or_create_role_atomic_goal(Predicate<P> predicate, bool is_negated);
    RoleIntersection get_or_create_role_intersection(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                     ConstructorOrNonTerminal<Role> role_or_non_terminal_right);
    RoleUnion get_or_create_role_union(ConstructorOrNonTerminal<Role> role_or_non_terminal_left, ConstructorOrNonTerminal<Role> role_or_non_terminal_right);
    RoleComplement get_or_create_role_complement(ConstructorOrNonTerminal<Role> role_or_non_terminal);
    RoleInverse get_or_create_role_inverse(ConstructorOrNonTerminal<Role> role_or_non_terminal);
    RoleComposition get_or_create_role_composition(ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                                   ConstructorOrNonTerminal<Role> role_or_non_terminal_right);
    RoleTransitiveClosure get_or_create_role_transitive_closure(ConstructorOrNonTerminal<Role> role_or_non_terminal);
    RoleReflexiveTransitiveClosure get_or_create_role_reflexive_transitive_closure(ConstructorOrNonTerminal<Role> role_or_non_terminal);
    RoleRestriction get_or_create_role_restriction(ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                                   ConstructorOrNonTerminal<Concept> concept_or_non_terminal);
    RoleIdentity get_or_create_role_identity(ConstructorOrNonTerminal<Concept> concept_or_non_terminal);
};

}

#endif
