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

namespace mimir::languages::dl::grammar
{

/**
 * Grammar
 */

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using NonTerminalRepository = loki::IndexedHashSet<NonTerminalImpl<D>>;

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using ChoiceRepository = loki::IndexedHashSet<ConstructorOrNonTerminalImpl<D>>;

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
using DerivationRuleRepository = loki::IndexedHashSet<DerivationRuleImpl<D>>;

using ConceptBotRepository = loki::IndexedHashSet<ConceptBotImpl>;
using ConceptTopRepository = loki::IndexedHashSet<ConceptTopImpl>;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using ConceptAtomicStateRepositoryImpl = loki::IndexedHashSet<ConceptAtomicStateImpl<P>>;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using ConceptAtomicGoalRepository = loki::IndexedHashSet<ConceptAtomicGoalImpl<P>>;
using ConceptIntersectionRepository = loki::IndexedHashSet<ConceptIntersectionImpl>;
using ConceptUnionRepository = loki::IndexedHashSet<ConceptUnionImpl>;
using ConceptNegationRepository = loki::IndexedHashSet<ConceptNegationImpl>;
using ConceptValueRestrictionRepository = loki::IndexedHashSet<ConceptValueRestrictionImpl>;
using ConceptExistentialQuantificationRepository = loki::IndexedHashSet<ConceptExistentialQuantificationImpl>;
using ConceptRoleValueMapContainmentRepository = loki::IndexedHashSet<ConceptRoleValueMapContainmentImpl>;
using ConceptRoleValueMapEqualityRepository = loki::IndexedHashSet<ConceptRoleValueMapEqualityImpl>;
using ConceptNominalRepository = loki::IndexedHashSet<ConceptNominalImpl>;

using RoleUniversalRepository = loki::IndexedHashSet<RoleUniversalImpl>;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using RoleAtomicStateRepositoryImpl = loki::IndexedHashSet<RoleAtomicStateImpl<P>>;
template<formalism::IsStaticOrFluentOrDerivedTag P>
using RoleAtomicGoalRepository = loki::IndexedHashSet<RoleAtomicGoalImpl<P>>;
using RoleIntersectionRepository = loki::IndexedHashSet<RoleIntersectionImpl>;
using RoleUnionRepository = loki::IndexedHashSet<RoleUnionImpl>;
using RoleComplementRepository = loki::IndexedHashSet<RoleComplementImpl>;
using RoleInverseRepository = loki::IndexedHashSet<RoleInverseImpl>;
using RoleCompositionRepository = loki::IndexedHashSet<RoleCompositionImpl>;
using RoleTransitiveClosureRepository = loki::IndexedHashSet<RoleTransitiveClosureImpl>;
using RoleReflexiveTransitiveClosureRepository = loki::IndexedHashSet<RoleReflexiveTransitiveClosureImpl>;
using RoleRestrictionRepository = loki::IndexedHashSet<RoleRestrictionImpl>;
using RoleIdentityRepository = loki::IndexedHashSet<RoleIdentityImpl>;

template<formalism::IsStaticOrFluentOrDerivedTag P>
using BooleanAtomicStateRepository = loki::IndexedHashSet<BooleanAtomicStateImpl<P>>;
template<IsConceptOrRoleTag D>
using BooleanNonemptyRepository = loki::IndexedHashSet<BooleanNonemptyImpl<D>>;

template<IsConceptOrRoleTag D>
using NumericalCountRepository = loki::IndexedHashSet<NumericalCountImpl<D>>;
using NumericalDistanceRepository = loki::IndexedHashSet<NumericalDistanceImpl>;

using HanaRepositories = boost::hana::map<
    boost::hana::pair<boost::hana::type<NonTerminalImpl<ConceptTag>>, NonTerminalRepository<ConceptTag>>,
    boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<ConceptTag>>, ChoiceRepository<ConceptTag>>,
    boost::hana::pair<boost::hana::type<DerivationRuleImpl<ConceptTag>>, DerivationRuleRepository<ConceptTag>>,
    boost::hana::pair<boost::hana::type<ConceptBotImpl>, ConceptBotRepository>,
    boost::hana::pair<boost::hana::type<ConceptTopImpl>, ConceptTopRepository>,
    boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::StaticTag>>, ConceptAtomicStateRepositoryImpl<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::FluentTag>>, ConceptAtomicStateRepositoryImpl<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicStateImpl<formalism::DerivedTag>>, ConceptAtomicStateRepositoryImpl<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::StaticTag>>, ConceptAtomicGoalRepository<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::FluentTag>>, ConceptAtomicGoalRepository<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<ConceptAtomicGoalImpl<formalism::DerivedTag>>, ConceptAtomicGoalRepository<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<ConceptIntersectionImpl>, ConceptIntersectionRepository>,
    boost::hana::pair<boost::hana::type<ConceptUnionImpl>, ConceptUnionRepository>,
    boost::hana::pair<boost::hana::type<ConceptNegationImpl>, ConceptNegationRepository>,
    boost::hana::pair<boost::hana::type<ConceptValueRestrictionImpl>, ConceptValueRestrictionRepository>,
    boost::hana::pair<boost::hana::type<ConceptExistentialQuantificationImpl>, ConceptExistentialQuantificationRepository>,
    boost::hana::pair<boost::hana::type<ConceptRoleValueMapContainmentImpl>, ConceptRoleValueMapContainmentRepository>,
    boost::hana::pair<boost::hana::type<ConceptRoleValueMapEqualityImpl>, ConceptRoleValueMapEqualityRepository>,
    boost::hana::pair<boost::hana::type<ConceptNominalImpl>, ConceptNominalRepository>,
    boost::hana::pair<boost::hana::type<NonTerminalImpl<RoleTag>>, NonTerminalRepository<RoleTag>>,
    boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<RoleTag>>, ChoiceRepository<RoleTag>>,
    boost::hana::pair<boost::hana::type<DerivationRuleImpl<RoleTag>>, DerivationRuleRepository<RoleTag>>,
    boost::hana::pair<boost::hana::type<RoleUniversalImpl>, RoleUniversalRepository>,
    boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::StaticTag>>, RoleAtomicStateRepositoryImpl<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::FluentTag>>, RoleAtomicStateRepositoryImpl<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicStateImpl<formalism::DerivedTag>>, RoleAtomicStateRepositoryImpl<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::StaticTag>>, RoleAtomicGoalRepository<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::FluentTag>>, RoleAtomicGoalRepository<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<RoleAtomicGoalImpl<formalism::DerivedTag>>, RoleAtomicGoalRepository<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<RoleIntersectionImpl>, RoleIntersectionRepository>,
    boost::hana::pair<boost::hana::type<RoleUnionImpl>, RoleUnionRepository>,
    boost::hana::pair<boost::hana::type<RoleComplementImpl>, RoleComplementRepository>,
    boost::hana::pair<boost::hana::type<RoleInverseImpl>, RoleInverseRepository>,
    boost::hana::pair<boost::hana::type<RoleCompositionImpl>, RoleCompositionRepository>,
    boost::hana::pair<boost::hana::type<RoleTransitiveClosureImpl>, RoleTransitiveClosureRepository>,
    boost::hana::pair<boost::hana::type<RoleReflexiveTransitiveClosureImpl>, RoleReflexiveTransitiveClosureRepository>,
    boost::hana::pair<boost::hana::type<RoleRestrictionImpl>, RoleRestrictionRepository>,
    boost::hana::pair<boost::hana::type<RoleIdentityImpl>, RoleIdentityRepository>,
    boost::hana::pair<boost::hana::type<NonTerminalImpl<BooleanTag>>, NonTerminalRepository<BooleanTag>>,
    boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<BooleanTag>>, ChoiceRepository<BooleanTag>>,
    boost::hana::pair<boost::hana::type<DerivationRuleImpl<BooleanTag>>, DerivationRuleRepository<BooleanTag>>,
    boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::StaticTag>>, BooleanAtomicStateRepository<formalism::StaticTag>>,
    boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::FluentTag>>, BooleanAtomicStateRepository<formalism::FluentTag>>,
    boost::hana::pair<boost::hana::type<BooleanAtomicStateImpl<formalism::DerivedTag>>, BooleanAtomicStateRepository<formalism::DerivedTag>>,
    boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<ConceptTag>>, BooleanNonemptyRepository<ConceptTag>>,
    boost::hana::pair<boost::hana::type<BooleanNonemptyImpl<RoleTag>>, BooleanNonemptyRepository<RoleTag>>,
    boost::hana::pair<boost::hana::type<NonTerminalImpl<NumericalTag>>, NonTerminalRepository<NumericalTag>>,
    boost::hana::pair<boost::hana::type<ConstructorOrNonTerminalImpl<NumericalTag>>, ChoiceRepository<NumericalTag>>,
    boost::hana::pair<boost::hana::type<DerivationRuleImpl<NumericalTag>>, DerivationRuleRepository<NumericalTag>>,
    boost::hana::pair<boost::hana::type<NumericalCountImpl<ConceptTag>>, NumericalCountRepository<ConceptTag>>,
    boost::hana::pair<boost::hana::type<NumericalCountImpl<RoleTag>>, NumericalCountRepository<RoleTag>>,
    boost::hana::pair<boost::hana::type<NumericalDistanceImpl>, NumericalDistanceRepository>>;

class Repositories
{
private:
    HanaRepositories m_repositories;

public:
    Repositories() = default;
    Repositories(const Repositories& other) = delete;
    Repositories& operator=(const Repositories& other) = delete;
    Repositories(Repositories&& other) = default;
    Repositories& operator=(Repositories&& other) = default;

    HanaRepositories& get_repositories();
    const HanaRepositories& get_repositories() const;

    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    DerivationRule<D> get_or_create_derivation_rule(NonTerminal<D> non_terminal, ConstructorOrNonTerminalList<D> constructor_or_non_terminals);
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    NonTerminal<D> get_or_create_nonterminal(std::string name);
    template<IsConceptOrRoleOrBooleanOrNumericalTag D>
    ConstructorOrNonTerminal<D> get_or_create_constructor_or_nonterminal(std::variant<Constructor<D>, NonTerminal<D>> choice);

    /* Concepts */
    Constructor<ConceptTag> get_or_create_concept_bot();
    Constructor<ConceptTag> get_or_create_concept_top();
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<ConceptTag> get_or_create_concept_atomic_state(formalism::Predicate<P> predicate);
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<ConceptTag> get_or_create_concept_atomic_goal(formalism::Predicate<P> predicate, bool polarity);
    Constructor<ConceptTag> get_or_create_concept_intersection(ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                                               ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal);
    Constructor<ConceptTag> get_or_create_concept_union(ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                                        ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal);
    Constructor<ConceptTag> get_or_create_concept_negation(ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal);
    Constructor<ConceptTag> get_or_create_concept_value_restriction(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                                    ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal);
    Constructor<ConceptTag> get_or_create_concept_existential_quantification(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                                             ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal);
    Constructor<ConceptTag> get_or_create_concept_role_value_map_containment(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                                             ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal);
    Constructor<ConceptTag> get_or_create_concept_role_value_map_equality(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                                          ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal);
    Constructor<ConceptTag> get_or_create_concept_nominal(formalism::Object object);

    /* Roles */
    Constructor<RoleTag> get_or_create_role_universal();
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<RoleTag> get_or_create_role_atomic_state(formalism::Predicate<P> predicate);
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<RoleTag> get_or_create_role_atomic_goal(formalism::Predicate<P> predicate, bool polarity);
    Constructor<RoleTag> get_or_create_role_intersection(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                         ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_union(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                  ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_complement(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_inverse(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_composition(ConstructorOrNonTerminal<RoleTag> left_role_or_nonterminal,
                                                        ConstructorOrNonTerminal<RoleTag> right_role_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_transitive_closure(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_reflexive_transitive_closure(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_restriction(ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                        ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal);
    Constructor<RoleTag> get_or_create_role_identity(ConstructorOrNonTerminal<ConceptTag> concept_or_nonterminal);

    /* Booleans */
    template<formalism::IsStaticOrFluentOrDerivedTag P>
    Constructor<BooleanTag> get_or_create_boolean_atomic_state(formalism::Predicate<P> predicate);
    template<IsConceptOrRoleTag D>
    Constructor<BooleanTag> get_or_create_boolean_nonempty(ConstructorOrNonTerminal<D> constructor_or_nonterminal);

    /* Numericals */
    template<IsConceptOrRoleTag D>
    Constructor<NumericalTag> get_or_create_numerical_count(ConstructorOrNonTerminal<D> constructor_or_nonterminal);
    Constructor<NumericalTag> get_or_create_numerical_distance(ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                                               ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                                               ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal);
};

}

#endif
